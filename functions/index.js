const functions = require('firebase-functions');
const admin = require('firebase-admin');
// Initialize Firebase app
const serviceAccount = require('./key.json');
admin.initializeApp({
  credential: admin.credential.cert(serviceAccount)
});
const db = admin.firestore();
exports.getData = functions.https.onRequest(async (req, res) => {
  try {
    // Reference to the Firestore path
    const docRef = db.doc('/app/music');
    const doc = await docRef.get();
    // Check if the document exists
    if (doc.exists) {
      const data = doc.data();
      res.status(200).json({current: data.current});
    } else {
      res.status(404).json({
        status: 'error',
        message: 'Document not found'
      });
    }
  } catch (error) {
    res.status(500).json({
      status: 'error',
      message: error.message
    });
  }
});
// Make sure to replace '/path/to/your/serviceAccountKey.json' with the actual path to your Firebase Admin SDK JSON file.
