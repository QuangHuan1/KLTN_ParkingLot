const User = require('../models/User');

const UserController = {
    // [GET]
    index(req, res, next) {
        res.render('login/show');
    },

    // [POST] /sign-in
    signIn: async (req, res) => {
        try {
            const {username, password} = req.body;
            let user = await User.findOne({ username: username, password: password });
            if (user)
                user.password = null;
            return res.json(user);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    }
};

module.exports = UserController;
