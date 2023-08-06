const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const feeCoefficient = new Schema({
    registered: {type: Number},
    notRegistered: {type: Number},
});

module.exports = mongoose.model('fee_coefficients', feeCoefficient);
