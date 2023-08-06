const CarInfo = require('../models/CarInfo');
const Owner = require('../models/Owner');

const OwnerController = {
    // [GET] /owner
    index(req, res, next) {
        res.render('owner/show');
    },

    // [GET] /owner/data
    getDataOwners: async (req, res) => {
        try {
            let owners = await Owner.find({});
            owners = owners.map((item) => item.toObject());
            return res.json({ owners });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /owner/car/:slug
    getCarsOfOwner: async(req, res, next) => {
        let owner = await Owner.findOne({ _id: req.params.slug });
        owner = owner.toObject();
        res.render('owner/showCarOfOwner', {owner});
    },

    // [GET] /owner/data/:slug
    getDataOwner: async (req, res, next) => {
        try {
            let owner = await Owner.findOne({ _id: req.params.slug }).populate('carInfos');
            owner = owner.toObject();
            return res.json({ owner });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /owner/:slug
    getOwner: async (req, res, next) => {
        try {
            const owner = await Owner.findOne({ personalId: req.params.slug });
            return res.json({ owner });
        } catch (err) {
            return res.json({ msg: err.message });
        }   
    },    

    // [GET] /owner/create
    createOwner: async (req, res, next) => {
        res.render('owner/create');
    },

    // [POST] /owner/store
    storeOwner: async (req, res, next) => {
        try {
            // Check if owner already exists
            const validation = await OwnerController.validateExisted(req.body, 'post'); 
            if (validation != 'absent')
                return res.json(validation)

            var owner = new Owner(req.body);
            owner
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /owner/update/:slug
    updateOwner: async (req, res, next) => {
        try {
            console.log("[PUT] /owner/update/:slug")
            // Check if owner already exists
            const validation = await OwnerController.validateExisted(req.body, 'update', req.params.slug);
            if (validation != 'absent')
                return res.json(validation)

            await Owner.updateOne({_id: req.params.slug}, req.body)
                .catch(next);
            res.json('absent');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /owner/delete/:id
    deleteOwner: async (req, res, next) => {
        try {
            // Check if owner has cars (not be soft delete)
            const owner = await Owner.findOne({_id: req.params.id}).populate('carInfos');
            if (owner.carInfos[0])
                return res.json({car: 'present'});
            
            await Owner.delete({_id: req.params.id})
                .catch(next);

            return res.json({car: 'absent'});
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    //function validate
    validateExisted: async (owner, method, id) => {
        const oldOwner = await Owner.findOneWithDeleted({personalId: owner.personalId})
        
        // Check if personalId changes, if not update
        if (method == 'update')
            if (!oldOwner || oldOwner._id == id)
                return "absent"

        if(oldOwner && oldOwner.deleted) {

            oldOwner.name = owner.name;
            oldOwner.deleted = false;

            if (method == 'post') oldOwner.save();
            return "presentDeleted"
        }

        if(oldOwner && !oldOwner.deleted) {
            return "present"
        }

        return "absent"
    }
};

module.exports = OwnerController;
