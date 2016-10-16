'''
PEONY SERVER:
	Listen for push notifications
	on Web APIs and relay to
	curie Geniuno board
'''
from twilio.rest import TwilioRestClient
from pymongo import MongoClient #DB interface
import birdy.twitter as tw	# twitter client library
import flask as fl
import logging
import urllib2
import json
import sys
import os
sys.path.append(os.getcwd())

app = fl.Flask(__name__)

MONGO_URI = 'mongodb://heroku_478jfqc4:n5hhn4e7h20kqb8qv5cde9l3hn@ds033126.mlab.com:33126/heroku_478jfqc4'
MONGO_DB = 'heroku_478jfqc4'

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

def get_collection(colName):
	# Avoiding redundant connections
	print "Connecting to DB."
	if colName == 'ohrm':
		col = getattr(fl.g, '_colOHRM', None)
		if col is None:
			mongo_client = MongoClient(MONGO_URI)
			peony_db = mongo_client[MONGO_DB]
			# mongo_client = MongoClient()  #Local use only
			# peony_db = mongo_client['peonyData']
			col = fl.g._colOHRM = peony_db[colName]
	elif colName == 'gyro':
		col = getattr(fl.g, '_colGyro', None)
		if col is None:
			mongo_client = MongoClient(MONGO_URI)
			peony_db = mongo_client[MONGO_DB]
			# mongo_client = MongoClient()  #Local use only
			# peony_db = mongo_client['peonyData']
			col = fl.g._colGyro = peony_db[colName]

	return col

def get_collection_data(collection, query):
	jsonQuery = json.loads(query)
	data = collection.find(jsonQuery)	# List of matched results from db
	print data[0] 	# DEBUG
	return data

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

def store_data(collection, data):
	res = collection.insert_one(data)

	if getattr(res, "acknowledged") == True:
		msg = 'peony data saved successfully.'
		print msg
	else:
		msg = 'Failed to store data to peony data collection'
		print msg

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
	account_sid = "XXXXXXXXXXXXXXXXXX" # Your Account SID from www.twilio.com/console
	auth_token  = "XXXXXXXXXXXXXXXXXX"  # Your Auth Token from www.twilio.com/console
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

@app.route('/sensorDataInterface', methods = ['GET', 'POST'])
def heart_rate_interface():
	collection = get_collection('pData')
	if fl.request.method == 'POST':
		data = fl.request.args.get('data')
		store_data(collection, data)
		return fl.redirect(fl.url_for('render_home_page'))
	elif fl.request.method == 'GET':
		q = fl.request.args.get('q')
		data = get_collection_data(collection, q)
		return data

if __name__ == '__main__':
	app.logger.addHandler(logging.StreamHandler(sys.stdout))
	app.logger.setLevel(logging.ERROR)
	app.run()
