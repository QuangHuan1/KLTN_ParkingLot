const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const Owner = new Schema({
    personalId: { type: String, default: '' },
    name: { type: String, default: '' },
    eTags: [{ type: String, default: '' }],
});

module.exports = mongoose.model('owners', Owner);
