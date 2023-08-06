const express = require('express');
const router = express.Router();
const carInfoController = require('../app/controllers/car-info-controller');
const authMiddleware = require('../app/controllers/auth-middleware');

router.get('/data', carInfoController.getDataCarInfo);
router.put('/update/:id', authMiddleware.admin, carInfoController.updateCarInfo);
router.delete('/delete/:id', authMiddleware.admin, carInfoController.deleteCarInfo);
router.get('/:eTag', carInfoController.getCar);
router.post('/store/:idOwner', authMiddleware.admin, carInfoController.storeCarInfo);
router.get('/create/:slug', carInfoController.createCarInfo);
router.get('/:id', carInfoController.editCarInfo);
router.get('/', carInfoController.index);

module.exports = router;
