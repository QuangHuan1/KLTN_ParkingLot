const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Reader = new Schema({
    readerCode: { type: String, default: '' },
    state: { type: String, default: '' },
    gateCode: { type: String, default: '' },
    areaCode: { type: String, default: 'MK00' },
    gate: { type: Schema.Types.ObjectId, ref: 'gates' },
    area: { type: Schema.Types.ObjectId, ref: 'areas' },
});

module.exports = mongoose.model('readers', Reader);
