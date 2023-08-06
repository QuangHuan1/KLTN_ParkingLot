const Camera = require('../models/Camera');
const Sensor = require('../models/Sensor');
const Reader = require('../models/Reader');
const Gate = require('../models/Gate');
const Area = require('../models/Area');

const deviceController = {
    // [GET] /device
    getDevices: async (req, res) => {
        try {
            return res.render('device/show');
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /device/data
    getDataDevices: async (req, res) => {
        try {
            let cameras = await Camera.find({})
                .populate('gate');
            cameras = cameras.map((item) => item.toObject());
            cameras = cameras.map((item) => {
                return {
                    ...item,
                    deviceCode: item.cameraCode,
                    deviceType: 'Camera',
                    placeCode: item.gate.gateCode,
                };
            });

            let sensors = await Sensor.find({})
                .populate('gate')
                .populate('area');
            sensors = sensors.map((item) => item.toObject());
            sensors = sensors.map((item) => {
                return {
                    ...item,
                    deviceCode: item.sensorCode,
                    deviceType: 'Sensor',
                    placeCode: item.gate
                        ? item.gate.gateCode
                        : item.area.areaCode,
                };
            });

            let readers = await Reader.find({})
                .populate('gate')
                .populate('area');
            readers = readers.map((item) => item.toObject());
            readers = readers.map((item) => {
                return {
                    ...item,
                    deviceCode: item.readerCode,
                    deviceType: 'Reader',
                    placeCode: item.gate
                        ? item.gate.gateCode
                        : item.area.areaCode,
                };
            });

            let items = [];
            items = items.concat(cameras, sensors, readers);
            
            return res.json({ items });
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [GET] /device/deviceType/:deviceCode
    getDevice: async (req, res) => {
        try {
            const deviceType = req.params.deviceType;
            if (deviceType == 'camera')
            {
                let camera = await Camera.findOne({cameraCode: req.params.deviceCode})
                    .populate('gate');
                camera = camera.toObject()
                return res.json ({
                    ...camera,
                    deviceCode: camera.cameraCode,
                    deviceType: 'Camera',
                    placeCode: camera.gate.gateCode})
            }

            if (deviceType == 'sensor')
            {
                let sensor = await Sensor.findOne({sensorCode: req.params.deviceCode})
                    .populate('gate')
                    .populate('area');
                sensor = sensor.toObject()
                return res.json ({
                    ...sensor, 
                    deviceCode: sensor.sensorCode,
                    deviceType: 'Sensor',
                    placeCode: sensor.gate ? sensor.gate.gateCode : sensor.area.areaCode })
            }

            if (deviceType == 'reader')
            {
                let reader = await Reader.findOne({readerCode: req.params.deviceCode})
                    .populate('gate')
                    .populate('area');
                reader = reader.toObject()
                return res.json ({
                    ...reader, 
                    deviceCode: reader.readerCode,
                    deviceType: 'Reader',
                    placeCode: reader.gate ? reader.gate.gateCode : reader.area.areaCode })
            }
          
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /device/store/camera
    postCamera: async (req, res, next) => {
        try {
            const validation = await deviceController.validateExisted(req.body, 'camera', 'post'); 
            if (validation != 'absent')
                return res.json(validation)


            var camera = new Camera(req.body);

            const gate = await Gate.findOne({ gateCode: camera.gateCode });
            camera.gate = gate._id;

            camera
                .save()
                .then(() => res.send(camera))
                .catch(next);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /device/store/sensor
    postSensor: async (req, res, next) => {
        try {
            const validation = await deviceController.validateExisted(req.body, 'sensor', 'post'); 
            if (validation != 'absent')
                return res.json(validation)

            var sensor = new Sensor(req.body);

            const gate = await Gate.findOne({ gateCode: sensor.gateCode });
            if (gate) sensor.gate = gate._id;

            const area = await Area.findOne({ areaCode: sensor.areaCode });
            if (area) sensor.area = area._id;

            sensor
                .save()
                .then(() => res.send(sensor))
                .catch(next);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [POST] /device/store/reader
    postReader: async (req, res, next) => {
        try {
            const validation = await deviceController.validateExisted(req.body, 'reader', 'post'); 
            if (validation != 'absent')
                return res.json(validation)

            var reader = new Reader(req.body);

            const gate = await Gate.findOne({ gateCode: reader.gateCode });
            if (gate) reader.gate = gate._id;

            const area = await Area.findOne({ areaCode: reader.areaCode });
            if (area) reader.area = area._id;

            reader
                .save()
                .then(() => res.send(reader))
                .catch(next);
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /device/update/camera/:id
    updateCamera: async (req, res, next) => {
        try {
            console.log('[PUT] /device/camera/:id')
            var camera = await Camera.findOne({_id: req.params.id})
            
            camera.state = req.body.state;

            const gate = await Gate.findOne({ gateCode: req.body.gateCode });
            camera.gate = gate._id;

            console.log(camera)
            camera
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /device/update/sensor/:id
    updateSensor: async (req, res, next) => {
        try {
            var sensor = await Sensor.findOne({_id: req.params.id})
            
            sensor.state = req.body.state;

            const gate = await Gate.findOne({ gateCode: req.body.gateCode });
            if (gate) sensor.gate = gate._id;

            const area = await Area.findOne({ areaCode: req.body.areaCode });
            if (area) sensor.gate = gate._id;


            sensor
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [PUT] /device/update/reader/:id
    updateReader: async (req, res, next) => {
        try {
            var reader = await Reader.findOne({_id: req.params.id})
            
            reader.state = req.body.state;

            const gate = await Gate.findOne({ gateCode: req.body.gateCode });
            if (gate) reader.gate = gate._id;

            const area = await Area.findOne({ areaCode: req.body.areaCode });
            if (area) reader.gate = gate._id;


            reader
                .save()
                .catch(next);
            res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /device/delete/camera/:id
    deleteCamera: async (req, res, next) => {
        try {
            await Camera.deleteOne({_id: req.params.id})
                .catch(next);
            return res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /device/delete/sensor/:id
    deleteSensor: async (req, res, next) => {
        try {
            console.log(req.params.id)
            console.log("[DELETE] /device/delete/sensor/:id")
            await Sensor.deleteOne({_id: req.params.id})
                .catch(next);
            return res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    // [DELETE] /device/delete/reader/:id
    deleteReader: async (req, res, next) => {
        try {
            await Reader.deleteOne({_id: req.params.id})
                .catch(next);
            return res.json({})
        } catch (err) {
            return res.json({ msg: err.message });
        }
    },

    //function validate
    validateExisted: async (device, deviceType) => {
        console.log("validateExisted")
        console.log(device)

        if (deviceType == 'sensor')
        {
            const oldDevice = await Sensor.findOne({sensorCode: device.sensorCode})
            if (oldDevice)
                return "present"
        }
        
        return "absent"
    }
};

module.exports = deviceController;
