const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Camera = new Schema({
    cameraCode: { type: String, default: '' },
    state: { type: String, default: '' },
    gateCode: { type: String, default: '' },
    gate: { type: Schema.Types.ObjectId, ref: 'gates' },
});

module.exports = mongoose.model('cameras', Camera);
