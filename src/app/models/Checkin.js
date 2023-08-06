const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Checkin = new Schema({
    eTag: { type: String, default: '' },
    gateCode: { type: String, default: '' },
    checkinTime: { type: Date, default: '' },
    areaCode: { type: String, default: 'MK00' },
    imageCode: { type: String, default: '' },
    carInfo: { type: Schema.Types.ObjectId, ref: 'car_infos' },
    gate: { type: Schema.Types.ObjectId, ref: 'gates' },
    area: { type: Schema.Types.ObjectId, ref: 'areas' },

    gateCodeOut: { type: String, default: '' },
    checkoutTime: { type: Date, default: '' },
    imageCodeOut: { type: String, default: '' },
    gateOut: { type: Schema.Types.ObjectId, ref: 'gates' },
});

module.exports = mongoose.model('check_ins', Checkin);
