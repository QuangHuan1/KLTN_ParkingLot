const User = require('../models/User');

const AuthMiddleware = {
    admin: async (req, res, next) => {
        try {
            var authusername = req.headers.authusername ? req.headers.authusername : req.headers['auth-username'];
            let user = await User.findOne({ username: authusername, role: 1});
            console.log(authusername)
            if (user){
                next();
            }
            else throw "admin not exist";
        } catch (err) {
            return res.json({ msg: err.message }); 
        }
    },
    tech: async (req, res, next) => {
        try {
            const {authusername} = req.headers;
            let user = await User.findOne({ username: authusername, role: 0});
            if (user)
                next();
            else throw "admin not exist";
        } catch (err) {
            return res.json({ msg: err.message });
        }
    }
};

module.exports = AuthMiddleware;
