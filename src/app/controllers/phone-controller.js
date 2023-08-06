const path = require('path');
const CarInfo = require('../models/CarInfo');
const Owner = require('../models/Owner');
const Checkin = require('../models/Checkin');

const CarInfoController = {
    /// [GET] /phone
    index: async(req, res, next) => {
        res.render('phone/show');
    },

    // [GET] /phone/:personalId
    getCarsOfOwner: async(req, res, next) => {
        let owner = await Owner.findOne({ personalId: req.params.personalId });
        if (!owner)
            owner = {}
        else {
            owner = owner.toObject();
        }
        res.render('phone/showCarOfOwner', {owner});
    },

    getAreaOfCar: async(req, res, next) => {
        let owner = await Owner.findOne({ personalId: req.params.personalId }).populate('carInfos');
        if (!owner)
            return res.json({});
        owner = owner.toObject();

        var areaOfCar = []
        for (car of owner.carInfos)
        {
            const checkin = await Checkin.findOne({carInfo: car, checkoutTime: ""})  
            areaOfCar.push(checkin ? checkin.areaCode : "Outside");
        }
        owner.areaOfCar = areaOfCar;
        return res.json({ owner });
    },
};

module.exports = CarInfoController;
