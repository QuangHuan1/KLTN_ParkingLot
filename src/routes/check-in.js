const express = require('express');
const router = express.Router();
const checkInController = require('../app/controllers/check-in-controller');

//[GET] /check-in
router.get('/', checkInController.getCheckin);

//[GET] /data
router.get('/data', checkInController.getDataCheckIns);

//[GET] /:id
router.get('/:id', checkInController.getDataCheckIn);

//[POST] /check-in
router.post('/', checkInController.index);

//[PUT] /check-in/check-in-area - to get in subdivision
router.put('/check-in-area', checkInController.checkinArea);

//[PUT] /check-in/check-out-area - to get out of subdivision
router.put('/check-out-area', checkInController.checkoutArea);

module.exports = router;
