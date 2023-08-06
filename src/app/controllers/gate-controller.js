const Gate = require('../models/Gate');

const GateController = {
    // [GET] /gate
    show(req, res, next) {
        res.render('gate/show');
    },

    // [GET] /gate/data
    getDataGates: async (req, res) => {
        try {
            let gates = await Gate.find({});
            gates = gates.map((item) => item.toObject());
            return res.json({ gates });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /gate/data/:gateCode
    getDataGate: async (req, res, next) => {
        try {
            let gate = await Gate.findOne({ gateCode: req.params.gateCode });
            gate = gate.toObject();
            return res.json({ gate });
        } catch (err) {
            return res.json({ msg: err.message });
        }   
    },  

    // [GET] /gate-tracking/:gateId
    trackGate: async (req, res) => {
        try {
            return res.render('gate/confirmCheck');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /gate/store
    storeGate: async (req, res, next) => {
        try {
            // Check if gate already exists
            const validation = await GateController.validateExisted(req.body, 'post'); 
            if (validation != 'absent')
                return res.json(validation)

            var gate = new Gate(req.body);
            gate
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /gate/update/:gateId
    updateGate: async (req, res, next) => {
        try {
            await Gate.updateOne({_id: req.params.gateId}, req.body)
                .catch(next);
            res.json("");
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /gate/delete/:gateId
    deleteGate: async (req, res, next) => {
        try {
            // Check if area has cars
            const gate = await Gate.findOne({_id: req.params.gateId});
            
            await gate.delete({_id: req.params.gateId})
                .catch(next);

            return res.json("");
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    //function validate
    validateExisted: async (gate, method, id) => {
        const oldGate = await Gate.findOneWithDeleted({gateCode: gate.gateCode})
        
        if(oldGate && oldGate.deleted) {

            oldGate.state = gate.state;
            oldGate.deleted = false;

            oldGate.save();
            return "presentDeleted"
        }

        if(oldGate && !oldGate.deleted) {
            return "present"
        }

        return "absent"
    }
};

module.exports = GateController;
