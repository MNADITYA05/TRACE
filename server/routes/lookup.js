const express = require('express');
const router = express.Router();
const { lookupBarcode } = require('../controllers/barcodeController');

router.post('/', lookupBarcode);

module.exports = router;
