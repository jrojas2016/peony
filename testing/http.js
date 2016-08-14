

function httpGet(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, true ); // false for synchronous request
    xmlHttp.send( null );
    // console.log(xmlHttp.responseText)
    return xmlHttp.responseText;
}

var res = httpGet("http://peony-curie.herokuapp.com/getPushNotifications")
console.log(res)