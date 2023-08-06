const Checkin = require('../models/Checkin');
const Area = require('../models/Area');
const Gate = require('../models/Gate');
const CarInfo = require('../models/CarInfo');
const defaultAreaCode = 'MK00';

const CheckinController = {
    // [GET] /check-in
    getCheckin: async (req, res) => {
        try {
            return res.render('checkIn/checkIn');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /check-in/data
    getDataCheckIns: async (req, res) => {
        try {
            let checkIns = await Checkin.find({})
                .populate('carInfo')
                .populate('gate')
                .populate('gateOut')
                .populate('area');
            checkIns = checkIns.map((item) => item.toObject());
            return res.json({ checkIns });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /check-in/:id
    getDataCheckIn: async (req, res) => {
        try {
            let checkIn = await Checkin.findOne({_id: req.params.id})
                .populate('carInfo')
                .populate('gate')
                .populate('gateOut')
                .populate('area');
                checkIn = checkIn.toObject();
            return res.json({ checkIn });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /check-in
    index: async (req, res, next) => {
        try {
            // check if check-in present
            var oldCheckin = await Checkin.findOne({ eTag: req.body.eTag, checkoutTime: null});
            if(oldCheckin) {
                req.app.io.emit('alarm', {message: "Multiple check-ins"});
                await fetch('http://localhost:3000/check-out', { //FIX relative address
                    method: 'POST',
                    body: JSON.stringify({ 
                        eTag: req.body.eTag, 
                        checkoutTime: req.body.checkinTime,
                        gateCode: 'MC01',
                        imageCode: 'none',
                        realAreaCode: oldCheckin.areaCode }),
                    headers: {
                        'Content-type': 'application/json; charset=UTF-8',
                        'Auth-username': 'admin'
                    },
                })
                    .then((response) => response.json())
                    .catch((err) => console.log(err));
            }

            // Checkin
            var checkin = new Checkin(req.body);

            if (!req.body.checkinTime)
                checkin.checkinTime = new Date(Date.now());

            // Assign ID of car, gate, area to check-in
            var carInfo = await CarInfo.findOne({ eTag: checkin.eTag });
            
            // For cars have not yet to enter car parking
            if (!carInfo) {
                await fetch('http://localhost:3000/car-info/store/000000000000000000000000', { //FIX relative address
                    method: 'POST',
                    body: JSON.stringify({ eTag: checkin.eTag }),
                    headers: {
                        'Content-type': 'application/json; charset=UTF-8',
                        'Auth-username': 'admin'
                    },
                })
                    .then((response) => response.json())
                    .then((json) => (carInfo = json))
                    .catch((err) => console.log(err));
            }
            checkin.carInfo = carInfo._id;

            const gate = await Gate.findOne({ gateCode: checkin.gateCode });
            checkin.gate = gate._id;

            const area = await Area.findOne({ areaCode: checkin.areaCode });
            checkin.area = area._id;

            // Insert the car into default Area
            area.carInfos.push(checkin.carInfo);
            area.eTags.push(checkin.eTag);
            area.vacancy = area.vacancy - 1;
            area.save();

            const confirmPendingCheck = {
                eTag: checkin.eTag,
                licenseNum: carInfo.licenseNum,
                type: carInfo.type,
                gateCode: gate.gateCode,
                checkTime: checkin.checkinTime,
                imageCode: checkin.imageCode,
                checkin: checkin,
            }; 

            req.app.io.emit('confirmPendingCheck', confirmPendingCheck);

            checkin
                .save()
                .then(() => res.send(checkin))
                .catch(next);
            
            // Create fake check-in area
            await fetch('http://localhost:3000/check-in/check-in-area', { //FIX relative address
                    method: 'PUT',
                    body: JSON.stringify({ eTag: checkin.eTag, areaCode: req.body.realAreaCode}),
                    headers: {
                        'Content-type': 'application/json; charset=UTF-8',
                        'Auth-username': 'admin'
                    },
                })
                    .then((response) => response.json())
                    .catch((err) => console.log(err));

            // Socket emit
            req.app.io.emit('newCheckIn', true);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /check-in/check-in-area
    checkinArea: async (req, res, next) => {
        try {
            console.log('[PUT] /check-in');

            var checkinArea = req.body;
            const area = await Area.findOne({ areaCode: checkinArea.areaCode });
            const carInfo = await CarInfo.findOne({ eTag: checkinArea.eTag });
            const carId = carInfo._id
            
            //const preUpdatedCheckin = await Checkin.findOne({ eTag: checkinArea.eTag });
            //const preUpdatedCheckinAreaCode = preUpdatedCheckin.areaCode;

            // Update Check-in
            Checkin.findOneAndUpdate(
                { eTag: checkinArea.eTag, checkoutTime: "" },
                { $set: { areaCode: checkinArea.areaCode, area: area._id } },
                (err, doc) => {
                    if (err) {
                        console.log('Something wrong when updating data!');
                    }
                    req.app.io.emit('newCheckIn', true);
                    res.send(doc);
                },
            );

            // Insert the car into compatible Area
            Area.findOne({ areaCode: checkinArea.areaCode })
                .then((area) => {
                    area.vacancy = area.vacancy - 1;
                    area.eTags.push(checkinArea.eTag);
                    area.carInfos.push(carInfo._id);
                    console.log(area.vacancy);
                    area.save();
                })
                .catch(next);

            // Delete the car from default Area
            Area.findOne({ areaCode: defaultAreaCode })
                .then((area) => {
                    area.vacancy = area.vacancy + 1;
                    area.eTags = area.eTags.filter(
                        (eTag) => eTag != checkinArea.eTag,
                    );
    
                    area.carInfos = area.carInfos.filter(
                        (item) => item.toString() != carId.toString(),
                    );
                    
                    area.save();
                })
                .catch(next);

        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /check-in/check-out-area
    checkoutArea: async (req, res, next) => {
        try {
            console.log('[PUT] /check-out');
            var checkoutArea = req.body;
            
            const area = await Area.findOne({ areaCode: checkoutArea.areaCode });
            const carInfo = await CarInfo.findOne({ eTag: checkoutArea.eTag });
            const carId = carInfo._id

            // Update Check-in
            const defaultArea = await Area.findOne({ areaCode: defaultAreaCode });
            Checkin.findOneAndUpdate(
                { eTag: checkoutArea.eTag, checkoutTime: "" },
                { $set: { areaCode: defaultAreaCode, area: defaultArea._id } },
                (err, doc) => {
                    if (err) {
                        console.log('Something wrong when updating data!');
                    }
                    req.app.io.emit('newCheckIn', true);
                    res.send(doc);
                },
            );
            // Insert the car into default Area
            Area.findOne({ areaCode: defaultAreaCode })
                .then((Area) => {
                    Area.vacancy = Area.vacancy - 1;
                    Area.eTags.push(checkoutArea.eTag);
                    Area.carInfos.push(carInfo._id);
                    Area.save();
                })
                .catch(next);

            // Delete the car from compatible Area
            Area.findOne({ areaCode: checkoutArea.areaCode })
                .then((Area) => {
                    Area.vacancy = Area.vacancy + 1;
                    Area.eTags = Area.eTags.filter(
                        (eTag) => eTag != checkoutArea.eTag,
                    );
                    Area.carInfos = Area.carInfos.filter(
                        (item) => item.toString() != carId.toString(), 
                    );

                    Area.save();
                })
                .catch(next);

        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /alarm
    alarm: async (req, res, next) => {
        try {
            const message = req.body
            req.app.io.emit('alarm', message);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },
};

module.exports = CheckinController;
