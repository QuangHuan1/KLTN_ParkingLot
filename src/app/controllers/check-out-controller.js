const Checkout = require('../models/Checkout');
const CarInfo = require('../models/CarInfo');
const defaultZoneCode = 'MK00';

const CheckoutController = {
    // [GET] /check-out
    getCheckout: async (req, res) => {
        try {
            let checkOuts = await Checkout.find({});
            const carInfos = await CarInfo.find({});
            checkOuts = checkOuts.map((item) => item.toObject());
            checkOuts = checkOuts.map((checkOut) => {
                const carInfo = carInfos.find(
                    (item) => item.eTag === checkOut.eTag,
                );
                return { ...checkOut, licenseNum: carInfo.licenseNum };
            });
            return res.render('checkOut', { checkOuts });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /check-out
    index(req, res, next) {
        var checkout = new Checkout(req.body);
        checkout.checkoutTime = new Date(Date.now());

        checkout
            .save()
            .then(() => res.send(checkout))
            .catch(next);
    },
};

module.exports = CheckoutController;
