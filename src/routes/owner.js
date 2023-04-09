const express = require('express');
const router = express.Router();
const ownerController = require('../app/controllers/owner-controller');

router.get('/:slug', ownerController.carOwner);
router.get('/', ownerController.index);

module.exports = router;
