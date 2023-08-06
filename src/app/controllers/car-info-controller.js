const path = require('path');
const CarInfo = require('../models/CarInfo');
const Owner = require('../models/Owner');
const Area = require('../models/Area');

const CarInfoController = {
    // [GET] /car-info
    index(req, res, next) {
        res.render('carInfo/show');
    },

    // [GET] /car-info/data
    getDataCarInfo: async (req, res) => {
        try {
            let carInfos = await CarInfo.find({}).populate('owner');
            carInfos = carInfos.map((item) => item.toObject());

            return res.json({ carInfos });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /car-info/:id
    editCarInfo: async(req, res, next) => {
        let carInfo = await CarInfo.findOne({ _id: req.params.id }).populate('owner');
        carInfo = carInfo.toObject();
        res.render('carInfo/edit', {carInfo});
    },

    // [GET] /car-info/create/:slug
    createCarInfo: async (req, res, next) => {
        let owner = await Owner.findOne({ _id: req.params.slug })
        owner = owner.toObject();
        res.render('carInfo/create', { owner });
    },
    
    // [GET] /car-info/:eTag
    getCar: async (req, res, next) => {
        try {
            const carInfo = await CarInfo.findOne({ eTag: req.params.eTag }).populate('owner');
            return res.json({ carInfo });
        } catch (err) {
            return res.json({ msg: err.message });
        }   
    }, 

    // [POST] /car-info/store/:idOwner
    storeCarInfo: async (req, res, next) => {
        try {
            // Check if car already exists
            console.log("[POST] /car-info/store")
            const validation = await CarInfoController.validateExisted(req.body, 'post'); 
            if (validation == 'present' || validation == 'presentDeleted')
                return res.json(validation)
            console.log(validation)

            let carInfo;
            if (validation != 'notRegistered')
                carInfo = new CarInfo(req.body);
            else
            {
                carInfo = await CarInfo.findOne({ eTag: req.body.eTag });
                carInfo.licenseNum = req.body.licenseNum;
                carInfo.type = req.body.type;
                carInfo.registered = 'R'
                carInfo.delete = false;
            }
                
            let owner = await Owner.findOne({ _id: req.params.idOwner });
            carInfo.owner = owner ? owner._id : null;

            // Insert the car into Owner
            if (owner) {
                owner.carInfos.push(carInfo._id);
                owner.eTags.push(carInfo.eTag);
                owner.save();
                carInfo.registered = 'R'
            }
            console.log(carInfo)
            carInfo
                .save()
                .catch(next);
            res.json(carInfo)
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /car-info/update/:id
    updateCarInfo: async (req, res, next) => {
        try {
            // Check if owner already exists
            const validation = await CarInfoController.validateExisted(req.body, 'update', req.params.id);
            if (validation != 'absent')
                return res.json(validation)
                
            await CarInfo.updateOne({_id: req.params.id}, {licenseNum: req.body.licenseNum, type: req.body.type})
                .catch(next);
            return res.json('absent')
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /car-info/delete/:id
    deleteCarInfo: async (req, res, next) => {
        try {
            // Check if area has cars
            const area = await Area.findOne({carInfos: req.params.id})
            if (area)
                return res.json({car: 'present'});

            await CarInfo.delete({_id: req.params.id})
                .catch(next);
            return res.json({car: 'absent'})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    //function validate
    validateExisted: async (carInfo, method, id) => {
        console.log("validateExisted")
        console.log(carInfo)
        // Have to Check etag and licenseNum
        //const oldCarInfo = !carInfo.licenseNum ? null : await CarInfo.findOneWithDeleted({licenseNum: carInfo.licenseNum})
        const oldCarInfo = !carInfo.licenseNum ? null : await CarInfo.findOneWithDeleted({eTag: carInfo.eTag})
        console.log(oldCarInfo)
        // Check if personalId changes, if not update        
        if (method == 'update')
            if (!oldCarInfo || oldCarInfo._id == id)
                return "absent"
            else
                return "present"

        if (oldCarInfo && oldCarInfo.registered == 'NY')
            return "notRegistered"

        if(oldCarInfo && oldCarInfo.deleted) {

            oldCarInfo.licenseNum = carInfo.licenseNum;
            oldCarInfo.type = carInfo.type;
            oldCarInfo.deleted = false;

            if (method == 'post') oldCarInfo.save();
            return "presentDeleted"
        }

        if(oldCarInfo && !oldCarInfo.deleted) {
            return "present"
        }

        return "absent"
    }
};

module.exports = CarInfoController;
