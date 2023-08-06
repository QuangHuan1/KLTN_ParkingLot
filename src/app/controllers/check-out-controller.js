const Checkin = require('../models/Checkin');
const Checkout = require('../models/Checkout');
const Area = require('../models/Area');
const Gate = require('../models/Gate');
const CarInfo = require('../models/CarInfo');
const Bill = require('../models/Bill');
const FeeCoefficient = require('../models/FeeCoefficient');

const defaultAreaCode = 'MK00';

const CheckoutController = {
    // [GET] /check-out
    getCheckout: async (req, res) => {
        try {
            return res.render('checkOut/checkOut');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /check-out/data
    getDataCheckOuts: async (req, res) => {
        try {
            let checkOuts = await Checkout.find({})
                .populate('carInfo')
                .populate('gate');
            checkOuts = checkOuts.map((item) => item.toObject());
            return res.json({ checkOuts });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /check-out/:id
    getDataCheckOut: async (req, res) => {
        try {
            let checkOut = await Checkout.findOne({_id: req.params.id})
                .populate('carInfo')
                .populate('gate');
            console.log(" [GET] /check-out/:id")
            checkOut = checkOut.toObject();
            return res.json({ checkOut });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /check-out
    index: async (req, res, next) => {
        try {

            // Check-out
            var checkout = req.body;
            if (!checkout.checkoutTime)
                checkout.checkoutTime = new Date(Date.now());
            
            // check if check-in present
            var checkin = await Checkin.findOne({ eTag: checkout.eTag, checkoutTime: null}).populate('carInfo')
            if (!checkin) 
            { 
                req.app.io.emit('alarm', {message: "Have not yet checked-in"});
                res.json("check-in not present");
                return;
            };

            // Create fake check-out area
            await fetch('http://localhost:3000/check-in/check-out-area', { //FIX relative address
                    method: 'PUT',
                    body: JSON.stringify({ eTag: req.body.eTag, areaCode: req.body.realAreaCode}),
                    headers: {
                        'Content-type': 'application/json; charset=UTF-8',
                        'Auth-username': 'admin'
                    },
                })
                    .then((response) => response.json())
                    .catch((err) => console.log(err));

            checkin.gateCodeOut = checkout.gateCode;
            checkin.checkoutTime = checkout.checkoutTime;
            checkin.imageCodeOut = checkout.imageCode;
            
            const carInfo = await CarInfo.findOne({ eTag: checkout.eTag});
            checkout.carInfo = carInfo._id;

            const gateOut = await Gate.findOne({ gateCode: checkout.gateCode });
            checkin.gateOut = gateOut._id;
            
            // Create Bill
            var parkingTime = Math.abs(new Date(checkout.checkoutTime) - new Date(checkin.checkinTime))
            var days = Math.floor(parkingTime/1000/60/60/24);
            var hours = Math.floor(parkingTime/1000/60/60) - days*24;
            var minutes = Math.floor(parkingTime/1000/60) - hours*60 - days*24*60;
            var seconds = Math.floor(parkingTime/1000) - minutes*60 - hours*60*60 - days*24*60*60;

            const feeCoefficient = await FeeCoefficient.findOne({});
            
            var fee;
            if (checkin.carInfo.registered === 'R')
                fee = (days*24 + hours) * feeCoefficient.registered;
            else
                fee = (days*24 + hours) * feeCoefficient.notRegistered;

            var bill = {
                eTag: checkin.eTag,
                checkinTime: checkin.checkinTime,
                checkoutTime: checkout.checkoutTime,
                parkingTime: {
                    days: days, hours : hours, minutes : minutes, seconds : seconds
                },
                fee: fee,
                checkInOut: checkin._id
            }


            bill = new Bill(bill)
            bill.save()

            checkin
                .save()
                .then(() => res.send(checkin))
                .catch(next);
            
            
            // Check confirm
            const confirmPendingCheck = {
                eTag: checkin.eTag,
                licenseNum: carInfo.licenseNum,
                type: carInfo.type,
                gateCode: checkin.gateCodeOut,
                checkTime: checkin.checkoutTime,
                imageCode: checkin.imageCodeOut
            }; 
            req.app.io.emit('confirmPendingCheck', confirmPendingCheck); 

            // Delete the car from default Area
            await Area.findOne({ areaCode: defaultAreaCode })
                .then((Area) => {
                    Area.vacancy = Area.vacancy + 1;
                    Area.eTags = Area.eTags.filter(
                        (eTag) => eTag != checkout.eTag,
                    );
                    Area.carInfos = Area.carInfos.filter((item) => {
                        item != carInfo._id;
                    });
                    console.log(Area.vacancy);
                    Area.save();
                })
                .catch(next);
                
            req.app.io.emit('newCheckout', true);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = CheckoutController;
