const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const CarInfo = new Schema({
    eTag: { type: String, default: '' },
    licenseNum: { type: String, default: '' },
    type: { type: String, default: '7' },
    registered: { type: String, default: 'NY' },
});

module.exports = mongoose.model('car_infos', CarInfo);
