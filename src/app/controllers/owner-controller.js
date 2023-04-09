const CarInfo = require('../models/CarInfo');
const Owner = require('../models/Owner');

class SiteController {
    // [GET] /owner
    index(req, res, next) {
        Owner.find({})
            .then((owners) => {
                owners = owners.map((owner) => owner.toObject());
                res.render('owner', { owners });
            })
            .catch(next);
    }

    // [GET] /owner/:slug
    carOwner(req, res, next) {
        Owner.findOne({ personalId: req.params.slug })
            .then((owner) => {
                // for(let i = 0; i < owner.eTags.length; i++) {
                //     CarInfo.findOne({eTag: owner.eTags[i]})
                //         .then((carInfo) => {
                //             console.log(carInfo)
                //         })
                //         .catch(next);

                CarInfo.find({ eTag: owner.eTags }) // Check it later
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
