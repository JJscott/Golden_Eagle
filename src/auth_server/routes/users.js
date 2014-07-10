var express = require('express');
var router = express.Router();

/* GET users listing. */
router.get('/login', function(req, res) {
	var uname = req.query.username;
	var pword = req.query.password;

	var Datastore = require('nedb')
  , db = new Datastore({ filename: 'users.nedb', autoload: true });

  	db.find({}, function(e, docs) {
  		console.log(docs);
  	});


  	console.log("username:'" + uname + "'");
  	db.findOne({ "username": uname }, function(err, doc) {
  		console.log("doc: ", doc);
  		if(doc != null) {
  			console.log("found record");
  			console.log(doc.password);
  			console.log(pword);
  			if(pword == doc.password) {
  				res.json({success: true, salt: doc.salt});
  			}
  		} else res.json({success: false});
  	});
});

module.exports = router;
