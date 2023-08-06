const CheckInOutController = {
    // [POST] /check-in-image/check-in
    postCheckinImage: async (req, res, next) => {
        try {
            const file = req.file;

            if (!file) {
                const error = new Error('Please upload a file');
                error.httpStatusCode = 400;
                return next(error);
            }

            res.json('Upload successful');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /check-out-image/check-out
    postCheckoutImage: async (req, res) => {
        try {
            const file = req.file;

            if (!file) {
                const error = new Error('Please upload a file');
                error.httpStatusCode = 400;
                return next(error);
            }

            res.json('Upload successful');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = CheckInOutController;
