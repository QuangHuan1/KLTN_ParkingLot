const confirmCheckinController = {
    // [GET] /confirm-check-in
    getConfirmCheckin: async (req, res) => {
        try {
            return res.render('confirmCheckin');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = confirmCheckinController;
