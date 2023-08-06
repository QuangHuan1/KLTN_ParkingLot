const mongoose = require('mongoose');
var mongooseDelete = require('mongoose-delete');

const Schema = mongoose.Schema;

const Gate = new Schema({
    gateCode: { type: String, default: '' },
    state: { type: String, default: '' },
});

Gate.plugin(mongooseDelete, { overrideMethods: 'all' });

module.exports = mongoose.model('gates', Gate);
