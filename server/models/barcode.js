const mongoose = require('mongoose');

const barcodeSchema = new mongoose.Schema({
  barcode:           { type: String, required: true, unique: true },
  batch_id:          { type: String },
  shift_id:          { type: String },
  place_id:          { type: String },
  manufacturing_date:{ type: String },
  quality_status:    { type: String },
  defect_type:       { type: String },
  operator_id:       { type: String },
  operator_name:     { type: String },
  timestamp:         { type: String },
  product_id:        { type: String },
  last_scanned_at:   { type: String, default: null }
});

module.exports = mongoose.model('Barcode', barcodeSchema);
