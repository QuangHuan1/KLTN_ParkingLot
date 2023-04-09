const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Area = new Schema({
    areaCode: { type: String, default: '' },
    vacancy: { type: Number, default: 0 },
    totalSlot: { type: Number, default: 0 },
    eTags: [{ type: String, default: '' }],
});

module.exports = mongoose.model('areas', Area);
