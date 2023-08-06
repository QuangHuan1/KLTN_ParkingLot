const path = require('path');
const Area = require('../models/Area');
const CarInfo = require('../models/CarInfo');

const DashboardController = {
    // [GET] /
    show(req, res, next) {
        res.render('dashboard/show');
    },

    // [GET] /getNumRegisteredCar
    getNumRegisteredCar: async (req, res) => {
        try {
            let registered = 0;
            let notRegistered = 0;
            let areas = await Area.find({}).populate('carInfos');
            areas = areas.map((item) => item.toObject());
            areas.forEach((area) => {
                const cars = area.carInfos;
                cars.forEach((car) => {
                    if (car.registered == 'R')   
                        registered++;
                    else
                        notRegistered++;
                })
            })
            const data = { registered, notRegistered }
            return res.json(data);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /getVacancies
    getVacancies: async (req, res) => {
        try {
            
            var area1 = await Area.findOne({areaCode: 'MK01'});
            var area2 = await Area.findOne({areaCode: 'MK02'});
            area1 = area1.toObject();
            area2 = area2.toObject();

            return res.json({ area1, area2 });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = DashboardController;
