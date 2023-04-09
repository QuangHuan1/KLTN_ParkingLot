const CarInfo = require('../models/CarInfo');
const Owner = require('../models/Owner');

class SiteController {
    // [GET] /
    index(req, res, next) {
        CarInfo.find({})
            .then((carInfos) => {
                carInfos = carInfos.map((carInfo) => carInfo.toObject());
                carInfos = carInfos.map((carInfo) => {
                    Owner.findOne({ eTags: carInfo.eTag })
                        .then((owner) => {
                            carInfo.owner = owner.name;
                        })
                        .catch(next);
                    return carInfo;
                });

                res.render('home', { carInfos });
            })
            .catch(next);
    }

    // [GET] /search
    search(req, res) {
        res.render('search');
    }
}

module.exports = new SiteController();
