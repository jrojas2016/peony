'''
PEONY SERVER:
	Listen for push notifications
	on Web APIs and relay to
	curie Geniuno board
'''

import birdy.twitter as tw	# twitter client library
import flask as fl
import logging
import urllib2
import json
import sys
import os
sys.path.append(os.getcwd())

app = fl.Flask(__name__)
app.logger.addHandler(logging.StreamHandler(sys.stdout))
app.logger.setLevel(logging.ERROR)

HOST_URL = 'http://localhost:5000/'	# local use only
# HOST_URL = 'http://peony-curie.herokuapp.com/getTwitterPush'
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

def save_access_token(twitterTokens, fileName):
	json.dump(twitterTokens, open('conf.json', 'w') )

def get_twitter_client(twitterTokens):
	'''
	Return a twitter app client instance
	'''
	consumerKey = twitterTokens['consumer_key']
	consumerSecret = twitterTokens['consumer_secret']
	accessToken = twitterTokens['access_tokens']
	if accessToken is None:
		client = tw.AppClient(consumerKey, consumerSecret)
		accessToken = client.get_access_token()
		twitterTokens['access_token'] = accessToken
		save_access_token(twitterTokens, TWITTER_CONF)
	else:
		client = tw.AppClient(consumerKey, consumerSecret, accessToken)

	return client

@app.before_first_request
def init():
	twClient = get_twitter_client(TWITTER_TOKENS)
	response = twClient.api.users.show.get(screen_name='twitter')
	print response.data

@app.route('/')
def render_home_page():
	return "<span>TWITTER API Activated</span>"

@app.route('/getPushNotifications')
def get_push_notifications():
	return "@twitterUser mentioned you in a tweet"

@app.route('/getFashionStatment')
def get_fashion_statement():
	pass

if __name__ == '__main__':
	app.logger.addHandler(logging.StreamHandler(sys.stdout))
	app.logger.setLevel(logging.ERROR)
	app.run(debug=True, use_reloader=False)
