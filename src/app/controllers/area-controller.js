const Area = require('../models/Area');
const CarInfo = require('../models/CarInfo');

const AreaController = {
    // [GET] /area
    index(req, res, next) {
        res.render('area/show');
    },

    // [GET] /area/data
    getDataAreas: async (req, res) => {
        try {
            let areas = await Area.find({});
            areas = areas.map((item) => item.toObject());
            return res.json({ areas });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /area/:slug
    showAreas(req, res, next) {
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
    },

    // [GET] /area/car/:areaCode
    getCarsInArea: async(req, res, next) => {
        let area = await Area.findOne({ _id: req.params.areaCode });
        area = area.toObject();
        res.render('area/showCarsInArea', {area});
    },
    
    // [GET] /area/data/:id
    getDataArea: async (req, res, next) => {
        try {
            let area = await Area.findOne({ _id: req.params.id }).populate('carInfos');
            area = area.toObject();
            return res.json({ area });
        } catch (err) {
            return res.json({ msg: err.message });
        }   
    },  

    // [GET] /area/:areaCode
    getArea: async (req, res, next) => {
        try {
            const area = await Area.findOne({ areaCode: req.params.areaCode });
            return res.json({ area });
        } catch (err) {
            return res.json({ msg: err.message });
        }   
    },  

    // [POST] /area/store
    storeArea: async (req, res, next) => {
        try {
            // Check if area already exists
            const validation = await AreaController.validateExisted(req.body, 'post'); 
            if (validation != 'absent')
                return res.json(validation)

            var area = new Area(req.body);
            area
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /area/update/:areaId
    updateArea: async (req, res, next) => {
        try {
            // Check if area already exists
            const validation = await AreaController.validateExisted(req.body, 'update', req.params.areaId);
            if (validation != 'absent')
                return res.json(validation)

            await Area.updateOne({_id: req.params.areaId}, req.body)
                .catch(next);
            res.json('absent');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /area/delete/:areaId
    deleteArea: async (req, res, next) => {
        try {
            // Check if area has cars
            console.log(req.params.areaId)
            const area = await Area.findOne({_id: req.params.areaId}).populate('carInfos');
            console.log(req.params.areaId)

            if (area.carInfos[0])
                return res.json({car: 'present'});
            
            await area.delete({_id: req.params.areaId})
                .catch(next);

            return res.json({car: 'absent'});
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    //function validate
    validateExisted: async (area, method, id) => {
        const oldArea = await Area.findOneWithDeleted({areaCode: area.areaCode})
        
        // Check if personalId changes, if not update
        if (method == 'update')
            if (!oldArea || oldArea._id == id)
                return "absent"

        if(oldArea && oldArea.deleted) {

            oldArea.vacancy = area.vacancy;
            oldArea.totalSlot = area.totalSlot;
            oldArea.deleted = false;

            if (method == 'post') oldArea.save();
            return "presentDeleted"
        }

        if(oldArea && !oldArea.deleted) {
            return "present"
        }

        return "absent"
    }
};

module.exports = AreaController;
