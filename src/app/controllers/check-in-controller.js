const Checkin = require('../models/Checkin');
const Area = require('../models/Area');
const CarInfo = require('../models/CarInfo');
const defaultZoneCode = 'MK00';

const CheckinController = {
    // [GET] /check-in
    getCheckin: async (req, res) => {
        try {
            let checkIns = await Checkin.find({});
            const carInfos = await CarInfo.find({});
            checkIns = checkIns.map((item) => item.toObject());
            checkIns = checkIns.map((checkIn) => {
                const carInfo = carInfos.find(
                    (item) => item.eTag === checkIn.eTag,
                );
                return { ...checkIn, licenseNum: carInfo.licenseNum };
            });
            return res.render('checkIn', { checkIns });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /check-in
    index: (req, res, next) => {
        var checkin = new Checkin(req.body);
        checkin.checkinTime = new Date(Date.now());

        checkin
            .save()
            .then(() => res.send(checkin))
            .catch(next);
    },

    // [PUT] /check-in/check-in-area
    checkinArea: (req, res, next) => {
        console.log('[PUT] /check-in');
        Checkin.findOneAndUpdate(
            { eTag: req.body.eTag },
            { $set: { areaCode: req.body.areaCode } },
            { upsert: true, sort: { checkinTime: -1 } },
            (err, doc) => {
                if (err) {
                    console.log('Something wrong when updating data!');
                }
                res.send(doc);
            },
        );

        Area.findOne({ areaCode: req.body.areaCode })
            .then((Area) => {
                Area.vacancy = Area.vacancy - 1;
                Area.eTags.push(req.body.eTag);
                console.log(Area.vacancy);
                Area.save();
            })
            .catch(next);
    },

    // [PUT] /check-in/check-out-area
    checkoutArea: (req, res, next) => {
        Checkin.findOneAndUpdate(
            { eTag: req.body.eTag },
            { $set: { areaCode: defaultZoneCode } },
            { upsert: true, sort: { checkinTime: -1 } },
            (err, doc) => {
                if (err) {
                    console.log('Something wrong when updating data!');
                }
                res.send(doc);
            },
        );

        Area.findOne({ areaCode: req.body.areaCode })
            .then((Area) => {
                Area.vacancy = Area.vacancy + 1;
                Area.eTags = Area.eTags.filter((eTag) => eTag != req.body.eTag);
                console.log(Area.vacancy);
                Area.save();
            })
            .catch(next);
    },
};

module.exports = CheckinController;
