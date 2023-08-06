const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const User = new Schema({
    username: { type: String, default: 0 },
    password: { type: String, default: 0 },
    role: { type: Number }, // 0 Tech, 1 Admin 
});


module.exports = mongoose.model('users', User);
