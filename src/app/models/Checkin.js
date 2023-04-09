const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Checkin = new Schema({
    eTag: { type: String, default: '' },
    gateCode: { type: String, default: '' },
    checkinTime: { type: Date, default: '' },
    areaCode: { type: String, default: 'MK00' },
    imageCode: { type: String, default: '' },
});

module.exports = mongoose.model('check_ins', Checkin);
