const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Bill = new Schema({
    checkinTime: { type: Date, default: '' },
    checkoutTime: { type: Date, default: '' },
    parkingTime: {type: Object},
    fee: {type: Number},
    checkInOut: { type: Schema.Types.ObjectId, ref: 'check_ins' },
});

module.exports = mongoose.model('bills', Bill);
