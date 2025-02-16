CREATE TABLE `sensors` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `date` datetime DEFAULT CURRENT_TIMESTAMP,
  `scd30_temp` double(3,1) DEFAULT NULL,
  `scd30_co2` smallint unsigned DEFAULT NULL,
  `scd30_h` smallint DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB