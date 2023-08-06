const express = require('express');
const router = express.Router();
const deviceController = require('../app/controllers/device-controller'); //fix
const authMiddleware = require('../app/controllers/auth-middleware');


//[GET] /device
router.get('/', deviceController.getDevices);

//[GET] /device/data
router.get('/:deviceType/:deviceCode', deviceController.getDevice);

//[GET] /device/data
router.get('/data', deviceController.getDataDevices);

//[POST] /device/store/camera
router.post('/store/camera', authMiddleware.tech, deviceController.postCamera);

//[POST] /device/store/sensor
router.post('/store/sensor', authMiddleware.tech, deviceController.postSensor);

//[POST] /device/store/reader
router.post('/store/reader', authMiddleware.tech, deviceController.postReader);

//[PUT] /device/update/camera
router.put('/update/camera/:id', authMiddleware.tech, deviceController.updateCamera);

//[PUT] /device/update/camera
router.put('/update/sensor/:id', authMiddleware.tech, deviceController.updateSensor);

//[PUT] /device/update/camera
router.put('/update/reader/:id', authMiddleware.tech, deviceController.updateReader);

//[DELETE] /device/delete/camera/:id
router.delete('/delete/camera/:id', authMiddleware.tech, deviceController.deleteCamera);

//[DELETE] /device/delete/sensor/:id
router.delete('/delete/sensor/:id', authMiddleware.tech, deviceController.deleteSensor);

//[DELETE] /device/delete/reader/:id
router.delete('/delete/reader/:id', authMiddleware.tech, deviceController.deleteReader);


module.exports = router;
