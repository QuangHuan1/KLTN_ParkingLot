const Bill = require('../models/Bill');
const FeeCoefficient = require('../models/FeeCoefficient');

const { populate } = require('../models/Checkin');

const BillController = {
    // [GET] /bill
    show(req, res, next) {
        res.render('bill/show');
    },

    // [GET] /bill/data
    getDataBills: async (req, res) => {
        try {
            let bills = await Bill.find({}).populate('checkInOut')
            
            return res.json({ bills });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /bill/update
    update: async (req, res, next) => {
        try {
            console.log(req.body)
            await FeeCoefficient.updateOne({}, req.body)
                .catch(next);
            res.json({});
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /bill/fix
    fix: async (req, res) => {
        try {
            let bills = await Bill.find({}).populate('checkInOut')
            bills = bills.map((item) => item.toObject());
            bills = bills.map((item) => {
                console.log(123213213213)
                item.checkinTime = item.checkInOut.checkinTime;
                item.checkoutTime = item.checkInOut.checkoutTime;
                var parkingTime = Math.abs(new Date(item.checkInOut.checkoutTime) - new Date(item.checkInOut.checkinTime))

                var days = Math.floor(parkingTime/1000/60/60/24);
                var hours = Math.floor(parkingTime/1000/60/60) - days*24;
                var minutes = Math.floor(parkingTime/1000/60) - hours*60 - days*24*60;
                var seconds = Math.floor(parkingTime/1000) - minutes*60 - hours*60*60 - days*24*60*60;

                var fee = (days*24 + hours) * 15;

                item.parkingTime.days = days;
                item.parkingTime.hours = hours;
                item.parkingTime.minutes = minutes;
                item.parkingTime.seconds = seconds;
                item.fee = fee;
                item.checkInOut = item.checkInOut._id;
                return item;
            });

            //bills[0].save()

            return res.json({ bills });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = BillController;
