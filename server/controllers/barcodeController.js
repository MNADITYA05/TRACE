const axios = require('axios');
const Barcode = require('../models/barcode');

const ML_API_URL = process.env.ML_API_URL || 'http://localhost:8000';

const lookupBarcode = async (req, res) => {
  let { barcode } = req.body;

  // Sanitize barcode input
  barcode = String(barcode).trim().replace(/\s/g, '').replace(/[​-‍﻿]/g, '');

  console.log('📥 Received barcode:', barcode);

  if (!barcode || typeof barcode !== 'string' || barcode.length !== 13) {
    return res.status(400).json({ status: 'error', message: 'Barcode must be a 13-digit string' });
  }

  try {
    const record = await Barcode.findOneAndUpdate(
      { barcode },
      { last_scanned_at: new Date().toISOString() },
      { new: true }
    );

    if (!record) {
      console.warn('❌ Not found in DB:', barcode);
      return res.status(404).json({ status: 'not_found', message: 'Barcode not found' });
    }

    console.log('✅ Found:', record);

    try {
      const response = await axios.post(`${ML_API_URL}/trigger`, {
        product_id: record.product_id
      });

      console.log('📦 ML API response:', response.data);

      return res.json({
        status: 'found',
        barcode: record.barcode,
        product_id: record.product_id,
        manufacturing_date: record.manufacturing_date,
        ml_result: response.data
      });
    } catch (mlError) {
      console.error('❌ ML API trigger failed:', mlError.message);
      return res.status(500).json({ status: 'ml_error', message: 'ML API trigger failed' });
    }
  } catch (err) {
    console.error('❌ Lookup failed:', err);
    return res.status(500).json({ status: 'error', message: 'Internal server error' });
  }
};

module.exports = { lookupBarcode };
