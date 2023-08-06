const express = require('express');
const router = express.Router();
const multer = require('multer');

const storage = multer.diskStorage({
    destination: function (req, file, cb) {
        cb(null, './src/public/img/checkin/');
    },
    filename: function (req, file, cb) {
        cb(null, file.originalname);
    },
});

const storageCheckout = multer.diskStorage({
    destination: function (req, file, cb) {
        cb(null, './src/public/img/checkout/');
    },
    filename: function (req, file, cb) {
        cb(null, file.originalname);
    },
});
const upload = multer({ storage: storage });
const uploadCheckout = multer({ storage: storageCheckout });
const checkInOutImageController = require('../app/controllers/check-in-out-image-controller');

router.post(
    '/check-in',
    upload.single('image'),
    checkInOutImageController.postCheckinImage,
);

router.post(
    '/check-out',
    uploadCheckout.single('image'),
    checkInOutImageController.postCheckoutImage,
);

module.exports = router;
