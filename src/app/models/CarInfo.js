const mongoose = require('mongoose');
var mongooseDelete = require('mongoose-delete');

const Schema = mongoose.Schema;

const CarInfo = new Schema({
    eTag: { type: String, default: '' },
    licenseNum: { type: String, default: '' },
    type: { type: String, default: '7' },
    registered: { type: String, default: 'NY' },
    owner: { type: Schema.Types.ObjectId, ref: 'owners' },
});

CarInfo.plugin(mongooseDelete, { overrideMethods: 'all' });

module.exports = mongoose.model('car_infos', CarInfo);
