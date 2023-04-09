const Area = require('../models/Area');
const CarInfo = require('../models/CarInfo');

class SiteController {
    // [GET] /area
    index(req, res, next) {
        Area.find({})
            .then((areas) => {
                areas = areas.map((area) => area.toObject());
                res.render('area', { areas });
            })
            .catch(next);
    }

    // [GET] /area/:slug
    showCars(req, res, next) {
        Area.findOne({ areaCode: req.params.slug })
            .then((area) => {
                CarInfo.find({ eTag: area.eTags }) // Check it later
                    .then((carInfos) => {
                        carInfos = carInfos.map((carInfo) =>
                            carInfo.toObject(),
                        );
                        res.render('carOwner', { carInfos });
                    })
                    .catch(next);
            })
            .catch(next);
    }
}

module.exports = new SiteController();
