'''
PEONY SERVER:
	Listen for push notifications
	on Web APIs and relay to
	curie Geniuno board
'''
from twilio.rest import TwilioRestClient
import birdy.twitter as tw	# twitter client library
import flask as fl
import logging
import urllib2
import json
import sys
import os
sys.path.append(os.getcwd())

app = fl.Flask(__name__)

HOST_URL = 'http://localhost:5000/'	# local use only
# HOST_URL = 'http://peony-curie.herokuapp.com/'
'''TWITTER API'''
TWITTER_CONF = 'conf.json'
TWITTER_TOKENS = json.load( open(TWITTER_CONF) )

def curl( url, data = None, authToken = None ):

	if data is not None:
		req = urllib2.Request( url, data )
	else:
		req = urllib2.Request( url )

	if authToken is not None:
		req.add_header( 'Authorization', 'Basic %s'%authToken )

	response = urllib2.urlopen( req )
	res = response.read()
	return res

def decode_notifications(notifList):
	pkgList = []
	for curr_notif in notifList:
		authorId = curr_notif['user']['id']
		# print type(authorId), authorId 	#DEBUG
		if authorId == 224360775:
			pkgList.append('-m1')
		elif authorId == 26443744:
			pkgList.append('-m2')
		elif authorId == 911629549:
			pkgList.append('-m3')
		else:
			pkgList.append('-m4')

	return pkgList

def get_twitter_client(twitterTokens):
	'''
	Return a twitter app client instance
	'''
	twClient = getattr(fl.g, '_twClient', None)
	if twClient is None:
		consumerKey = twitterTokens['consumer_key']
		consumerSecret = twitterTokens['consumer_secret']
		accessToken = twitterTokens['access_token']
		accessTokenSecret = twitterTokens['access_token_secret']
		twClient = fl.g._twClient = tw.UserClient(consumerKey, consumerSecret, accessToken, accessTokenSecret)

	return twClient

@app.before_first_request
def init():
	pass

@app.route('/')
def render_home_page():
	return "<span>PEONY SERVER ONLINE</span>"

@app.route('/getPushNotifications')
def get_push_notifications():
	pkg = {}
	twClient = get_twitter_client(TWITTER_TOKENS)
	response = twClient.api.statuses.mentions_timeline.get(count = 7, trim_user = 1)
	# print response.data	#DEBUG
	pkgList = decode_notifications(response.data)
	# print pkgList 	#DEBUG
	pkg['data'] = pkgList
	return json.dumps(pkg)

@app.route('/getFashionStatement', methods = [ 'GET','POST'])
def get_fashion_statement():
	return "#IoT is trending right now"

@app.route('/sendSMSAlert')
def send_sms_alert():
	account_sid = "AC8885646939013beffa55d1f57a4ad1a8" # Your Account SID from www.twilio.com/console
	auth_token  = "1ca2de59a66530e3cd20c43e365222e8"  # Your Auth Token from www.twilio.com/console
	longitude = fl.request.args.get('long')
	latitude = fl.request.args.get('lat')
	toNum = fl.request.args.get('to_num')

	client = TwilioRestClient(account_sid, auth_token)

	message = client.messages.create(
		body="Your friend Judy may be in danger!" +
			"Her last location was:\nhttp://maps.google.com/maps" +
			"?q={0},{1}&ll={0},{1}&z=17".format(latitude, longitude),
	    to="+1{0}".format(toNum),    # Replace with your phone number
    	from_="+16506810047") # Replace with your Twilio number

	# print(message.sid)	# DEBUG
	return fl.redirect(fl.url_for('render_home_page'))

if __name__ == '__main__':
	app.logger.addHandler(logging.StreamHandler(sys.stdout))
	app.logger.setLevel(logging.ERROR)
	app.run()
