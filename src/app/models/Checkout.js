const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Checkout = new Schema({
    eTag: { type: String, default: '' },
    gateCode: { type: String, default: '' },
    checkoutTime: { type: Date, default: '' },
    imageCode: { type: String, default: '' },
});

module.exports = mongoose.model('check_outs', Checkout);
