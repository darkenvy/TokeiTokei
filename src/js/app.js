// Get setPebble settings
var setPebbleURL = 'http://x.SetPebble.com/api/ZTMW/' + Pebble.getAccountToken();
var myAPIKey = "0";
var revAvailable = 0; // Declare it here so it can be accessed by the second xhrRequest scope

// ==================================================

// setup ajax request
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


// =================== Functions =====================

function getData() {
  console.log("inside getData");
  
  var url = "https://www.wanikani.com/api/user/" +
      myAPIKey + "/critical-items/95";// Construct URL
  var revURL = "https://www.wanikani.com/api/user/" +
      myAPIKey + "/study-queue";// This URL is for the review count URL only

  console.log("URL: " + url);
  console.log("revURL: " + revURL);
  
  // Send Review Count request to WaniKani
  xhrRequest(revURL, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);// responseText contains a JSON object with info
      revAvailable = json.requested_information.reviews_available; 
      console.log("Review Count (inside): " + revAvailable);
      
  });
  
  // Send regular equest to WaniKani
  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);// responseText contains a JSON object with info
      
      
      var rInt = Math.round(
        Math.random() * (json.requested_information.length -1) );
      //var rInt = 409; // 342 = kanji onyomi, 313 = kanji kunyomi, 409 is blue?
      console.log("rInt: " + rInt);
      
      var character = json.requested_information[rInt].character; 
      console.log("Character is: " + character);
      var meaning;
      try {
        meaning = json.requested_information[rInt].meaning.split(",")[0];
      } 
      catch (err) {
        console.log("could not split. Safely caught", err);  
      }
      console.log("Meaning is: " + meaning);
      var wType = (json.requested_information[rInt].type.charCodeAt(0) - 97) % 9; // Convert type from the word -> letter -> 1byte int
      console.log("Type is: " + wType); // 8 1 3
      var kana = "";
      var tmpStrip = "";
      
      // +++++++++ Kana is the var that is passed to watch no matter what ++++++++++++
      // +++++ So if the card is "kanji" and wants onyomi/kunyomi set kana to it +++++
      
      // Check if type is kanji, if yes, choose the important reading. If not, set to "kana"
      if(json.requested_information[rInt].type == "kanji") {
        console.log(rInt + " is kanji");
        if(json.requested_information[rInt].important_reading == "onyomi") {
          console.log(rInt + " is onyomi reading");
          tmpStrip = json.requested_information[rInt].onyomi.split(",");
          kana = tmpStrip[0];
          console.log("Strip[0]: " + kana);}
        else {
          console.log(rInt + " is kunyomi reading");
          tmpStrip = json.requested_information[rInt].kunyomi.split(",");
          kana = tmpStrip[0];
          console.log("Strip[0]: " + kana);}} 
      else {
        kana = json.requested_information[rInt].kana.split(",")[0];
        console.log("Kana is: " + kana);}
      
      // ++++++++ Check if any string is too long for the packet size +++++++++
      try {
        if(character.length > 10){
          character = character.substring(0,9);}}
      catch(err) {character = "";}
      
      try {
        if(kana.length > 10){
          kana = kana.substring(0,9);}}
      catch(err) {kana = "";}
      
      try {
        if(meaning.length > 32){
          meaning = meaning.substring(0,31);}}
      catch(err) {meaning = "";}
      
      
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_CHARACTER": character,
        "KEY_KANA": kana,
        "KEY_MEANING": meaning,
        "KEY_TYPE": wType,
        "KEY_REV": revAvailable
      };

      
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("WK Info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending WK info to Pebble!");
        }
      );
    }      
  );
}

// This is a cosmetic function
function informBlankAPI(){
//   console.log("RAN informBlankAPI()");
  var dictionary = {
        "KEY_CHARACTER": "鰐蟹",
        "KEY_KANA": "わにかに",
        "KEY_MEANING": "Set API Key",
        "KEY_TYPE": "8",
        "KEY_REV": 0
      };
  
  // Send to Pebble
  Pebble.sendAppMessage(dictionary, function(e) {
    console.log("WK sent API Noti to Pebble successfully!");
  }, function(e) {
    console.log("Error sending WK API Noti to Pebble!");
  });
}

// =================== Listeners =====================

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  console.log("PebbleKit JS ready!");
  
  // Always grab from setPebble, every time.
  xhrRequest(setPebbleURL, 'GET', function(responseText) {
    myAPIKey = JSON.parse(responseText)[1];// responseText contains a JSON object with info 
    console.log("Grabbed API Key from SetPebble: " + myAPIKey);

    // Save API key to localStorage, If greater than length 4 (default is "0")
    if(myAPIKey.length > 4) {
      // Check to see if the API key changed.
      if (localStorage.getItem("apiKey") != myAPIKey) {
        localStorage.setItem("apiKey", myAPIKey);
        console.log('Saved to settings: ' + localStorage.getItem("apiKey"));
      } else { console.log('API key is still the same as in memory. Did not re-write'); }
    } else { 
      console.log("apiKey from Settings page was Empty. Did not set. Using from memory");
      myAPIKey = localStorage.getItem("apiKey");
    }

    console.log("running getData now()");
    getData();

  });
  
  // If the API Key is still not set, inform the user, else the watch is ready
  if(myAPIKey === null || myAPIKey === "0") { console.log("myAPIKey===null, informBlankAPI()"); informBlankAPI(); } 
  else { console.log("else running, getting getData()"); getData(); }
  
  
  // Listen for when an AppMessage is received
  // This event listener is nested here so the API key has a chance to get set
  Pebble.addEventListener('appmessage', function(e) {
    console.log("running appmessage event listener");
    getData();
  });
  
});
 
// Show config page
Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('http://x.setpebble.com/ZTMW/' + Pebble.getAccountToken() );
});

// Wait for config page to be done
// No longer works?
Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));

  // Prepare AppMessage payload
  myAPIKey = config_data.apiKey;
  console.log('Returned API key from settings: ' + myAPIKey);
  
  if(myAPIKey.length > 4) {
    localStorage.setItem("apiKey", myAPIKey);
    console.log('Saved to settings: ' + localStorage.getItem("apiKey"));
  } else { console.log("apiKey from Settings page was Empty. Did not set"); }
  
  getData();

});
