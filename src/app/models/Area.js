const mongoose = require('mongoose');
var mongooseDelete = require('mongoose-delete');

const Schema = mongoose.Schema;

const Area = new Schema({
    areaCode: { type: String, default: '' },
    vacancy: { type: Number, default: 0 },
    totalSlot: { type: Number, default: 0 },
    eTags: [{ type: String, default: '' }],
    carInfos: [{ type: Schema.Types.ObjectId, ref: 'car_infos' }],
});

Area.plugin(mongooseDelete, { overrideMethods: 'all' });

module.exports = mongoose.model('areas', Area);
