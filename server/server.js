require('dotenv').config();
const express = require('express');
const bodyParser = require('body-parser');
const connectDB = require('./config/db');
const lookupRouter = require('./routes/lookup');

console.log('🛠 server.js started');

connectDB().catch(err => {
  console.error('❌ MongoDB connection error:', err);
  process.exit(1);
});

const app = express();
app.use(bodyParser.json());

app.get('/', (req, res) => {
  res.send('🔄 Barcode Lookup Server is Running');
});

app.use('/lookup', lookupRouter);

const PORT = process.env.PORT || 5050;
app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Server running on http://localhost:${PORT}`);
});
