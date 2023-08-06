const express = require('express');
const router = express.Router();
const phoneController = require('../app/controllers/phone-controller');

router.get('/:personalId', phoneController.getCarsOfOwner);
router.get('/:personalId/area-car', phoneController.getAreaOfCar);
router.get('/', phoneController.index);

module.exports = router;
