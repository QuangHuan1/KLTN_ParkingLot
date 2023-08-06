const mongoose = require('mongoose');
var mongooseDelete = require('mongoose-delete');

const Schema = mongoose.Schema;

const Owner = new Schema({
    personalId: { type: String, default: '' },
    name: { type: String, default: '' },
    eTags: [{ type: String, default: '' }],
    carInfos: [{ type: Schema.Types.ObjectId, ref: 'car_infos' }],
});

Owner.plugin(mongooseDelete, { overrideMethods: 'all' });

module.exports = mongoose.model('owners', Owner);
