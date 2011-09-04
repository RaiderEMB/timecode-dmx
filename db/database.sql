-- MySQL dump 10.13  Distrib 5.1.54, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: rdmx
-- ------------------------------------------------------
-- Server version	5.1.54-1ubuntu4

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `address`
--

DROP TABLE IF EXISTS `address`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `address` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `fixture` mediumint(8) unsigned NOT NULL,
  `offset` smallint(5) unsigned NOT NULL,
  `type` varchar(1) NOT NULL,
  `attr` varchar(1) NOT NULL,
  `bank` mediumint(9) NOT NULL,
  `name` varchar(255) NOT NULL,
  `onvalue` smallint(6) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unik` (`fixture`,`offset`),
  KEY `type` (`type`),
  KEY `attr` (`attr`),
  CONSTRAINT `address_ibfk_2` FOREIGN KEY (`attr`) REFERENCES `attr` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `address_ibfk_1` FOREIGN KEY (`fixture`) REFERENCES `fixture` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=18476 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `address`
--

LOCK TABLES `address` WRITE;
/*!40000 ALTER TABLE `address` DISABLE KEYS */;
/*!40000 ALTER TABLE `address` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `addressrange`
--

DROP TABLE IF EXISTS `addressrange`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `addressrange` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `address` mediumint(8) unsigned NOT NULL,
  `valfrom` smallint(5) unsigned NOT NULL,
  `valto` smallint(5) unsigned NOT NULL,
  `printf` varchar(255) NOT NULL,
  `formula` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `valfrom` (`valfrom`,`valto`),
  KEY `address` (`address`),
  CONSTRAINT `addressrange_ibfk_1` FOREIGN KEY (`address`) REFERENCES `address` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=53949 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `addressrange`
--

LOCK TABLES `addressrange` WRITE;
/*!40000 ALTER TABLE `addressrange` DISABLE KEYS */;
/*!40000 ALTER TABLE `addressrange` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `attr`
--

DROP TABLE IF EXISTS `attr`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `attr` (
  `id` varchar(1) NOT NULL,
  `name` varchar(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `attr`
--

LOCK TABLES `attr` WRITE;
/*!40000 ALTER TABLE `attr` DISABLE KEYS */;
INSERT INTO `attr` VALUES ('V','Blue'),('A','Color Wheel'),('N','Color Wheel 2'),('B','Cyan'),('H','Dimmer'),('R','Effect Rotate'),('L','Focus'),('S','Frost'),('I','Gobo'),('J','Gobo 2'),('M','Gobo 2 Rotate'),('K','Gobo Rotate'),('U','Green'),('G','Iris'),('C','Magenta'),('E','Pan'),('P','Prism'),('T','Red'),('O','Shutter'),('F','Tilt'),('0','Unknown'),('D','Yellow'),('Q','Zoom');
/*!40000 ALTER TABLE `attr` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `effect`
--

DROP TABLE IF EXISTS `effect`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `effect` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `effect`
--

LOCK TABLES `effect` WRITE;
/*!40000 ALTER TABLE `effect` DISABLE KEYS */;
/*!40000 ALTER TABLE `effect` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fixture`
--

DROP TABLE IF EXISTS `fixture`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fixture` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `brand` varchar(255) NOT NULL,
  `model` varchar(255) NOT NULL,
  `mode` varchar(128) DEFAULT NULL,
  `dmxsize` smallint(6) NOT NULL,
  `pandeg` smallint(5) unsigned NOT NULL,
  `tiltdeg` smallint(5) unsigned NOT NULL,
  `fixtype` varchar(40) NOT NULL,
  `fixshort` varchar(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `brand` (`brand`,`model`,`mode`,`dmxsize`),
  KEY `fixtype` (`fixtype`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fixture`
--

LOCK TABLES `fixture` WRITE;
/*!40000 ALTER TABLE `fixture` DISABLE KEYS */;
/*!40000 ALTER TABLE `fixture` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `patch`
--

DROP TABLE IF EXISTS `patch`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `patch` (
  `show` mediumint(8) unsigned NOT NULL,
  `dmx` smallint(5) unsigned NOT NULL,
  `fixture` mediumint(8) unsigned DEFAULT NULL,
  `tags` varchar(255) NOT NULL,
  PRIMARY KEY (`show`,`dmx`),
  KEY `fixture` (`fixture`),
  CONSTRAINT `patch_ibfk_2` FOREIGN KEY (`fixture`) REFERENCES `fixture` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `patch_ibfk_1` FOREIGN KEY (`show`) REFERENCES `show` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `patch`
--

LOCK TABLES `patch` WRITE;
/*!40000 ALTER TABLE `patch` DISABLE KEYS */;
/*!40000 ALTER TABLE `patch` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `show`
--

DROP TABLE IF EXISTS `show`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `show` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `show`
--

LOCK TABLES `show` WRITE;
/*!40000 ALTER TABLE `show` DISABLE KEYS */;
INSERT INTO `show` VALUES (1,'District2: Opening');
/*!40000 ALTER TABLE `show` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tc`
--

DROP TABLE IF EXISTS `tc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tc` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `show` mediumint(8) unsigned NOT NULL,
  `position` bigint(20) unsigned NOT NULL,
  `tags` varchar(255) NOT NULL,
  `comment` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `show` (`show`,`position`),
  KEY `group` (`tags`),
  CONSTRAINT `tc_ibfk_1` FOREIGN KEY (`show`) REFERENCES `show` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=182 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tc`
--

LOCK TABLES `tc` WRITE;
/*!40000 ALTER TABLE `tc` DISABLE KEYS */;
INSERT INTO `tc` VALUES (3,1,0,'Reset','DMX TAKEOVER LIGHTS STRAIGHT'),(4,1,60,'Lightsblue','ALL LIGHTS BLUE'),(5,1,119,'Lightsout','ALL LIGHTS OUT'),(6,1,249,'Music ',''),(7,1,533,'LowfreqFade ',''),(8,1,1050,'VocalFade',''),(9,1,1705,'VocalFade',''),(10,1,1820,'VocalUp',''),(11,1,2424,'VocalDown1',''),(12,1,2584,'VocalDown2',''),(13,1,2753,'VocalDown3',''),(14,1,3070,'Boom',''),(15,1,3135,'MaleVoice',''),(16,1,3588,'MaleVoice done',''),(17,1,3628,'DIS',''),(18,1,3656,'DIS',''),(19,1,3689,'DIS',''),(20,1,3746,'DIS,TRICT',''),(21,1,3844,'DIS1,Disnum',''),(22,1,3892,'DIS2,Disnum',''),(23,1,3951,'DIS3,Disnum',''),(24,1,4200,'BeatBar,Startbeat',''),(25,1,4270,'Takt',''),(26,1,4341,'Takt',''),(27,1,4373,'XX',''),(28,1,4409,'Takt',''),(29,1,4425,'XX',''),(30,1,4474,'BeatBar',''),(31,1,4544,'Takt',''),(32,1,4613,'Takt',''),(33,1,4684,'Takt',''),(34,1,4754,'BeatBar',''),(35,1,4823,'Takt',''),(36,1,4893,'Takt',''),(37,1,4924,'XX',''),(38,1,4958,'Takt',''),(39,1,4976,'XX',''),(40,1,5028,'BeatBar',''),(41,1,5097,'Takt',''),(42,1,5167,'Takt',''),(43,1,5236,'Takt',''),(44,1,5269,'PIPING',''),(45,1,5307,'BeatBar',''),(46,1,5375,'Takt',''),(47,1,5442,'Takt',''),(48,1,5476,'XX',''),(49,1,5510,'Takt',''),(50,1,5528,'XX',''),(51,1,5581,'BeatBar',''),(52,1,5649,'Takt',''),(53,1,5720,'Takt',''),(54,1,5788,'Takt',''),(55,1,5859,'BeatBar',''),(56,1,5923,'Takt',''),(57,1,5994,'Takt',''),(58,1,6028,'XX',''),(59,1,6063,'Takt',''),(60,1,6080,'XX',''),(61,1,6133,'BeatBar',''),(62,1,6202,'Takt',''),(63,1,6272,'Takt',''),(64,1,6341,'Takt',''),(65,1,6373,'Hihat',''),(66,1,6392,'Hihat',''),(67,1,6408,'BeatBar,Hovedparti',''),(68,1,6476,'Takt',''),(69,1,6511,'Piping',''),(70,1,6546,'Piping',''),(71,1,6547,'Takt',''),(72,1,6580,'XX',''),(73,1,6615,'Takt',''),(74,1,6632,'XX',''),(75,1,6686,'Bar','XX'),(76,1,6755,'Takt',''),(77,1,6825,'Takt',''),(78,1,6890,'Takt',''),(79,1,6958,'Bar','XX SMELL'),(80,1,7029,'Takt',''),(81,1,7099,'Takt',''),(82,1,7132,'XX',''),(83,1,7168,'Takt',''),(84,1,7184,'XX',''),(85,1,7239,'Bar',''),(86,1,7236,'XX',''),(87,1,7307,'Takt',''),(88,1,7372,'Takt',''),(89,1,7442,'Takt',''),(90,1,7511,'Bar','XX SMELL'),(91,1,7582,'Takt',''),(92,1,7652,'Takt',''),(93,1,7684,'XX',''),(94,1,7721,'Takt',''),(95,1,7736,'XX',''),(96,1,7791,'XX',''),(97,1,7856,'Takt',''),(98,1,7924,'Takt',''),(99,1,7995,'Takt',''),(100,1,8064,'Bar','SPJOINGSMELL'),(101,1,8134,'Takt',''),(102,1,8205,'Takt',''),(103,1,8273,'Takt',''),(104,1,8338,'Bar','Hovedparti takeoff'),(105,1,8408,'Takt',''),(106,1,8477,'Takt',''),(107,1,8547,'Takt',''),(108,1,8618,'Bar','Takeoff beats!'),(109,1,8687,'Takt',''),(110,1,8751,'Takt',''),(111,1,8822,'Takt',''),(112,1,8890,'Bar',''),(113,1,8961,'Takt',''),(114,1,9031,'Takt',''),(115,1,9100,'Takt',''),(116,1,9171,'Bar',''),(117,1,9235,'Takt',''),(118,1,9303,'Takt',''),(119,1,9374,'Takt',''),(120,1,9445,'Bar',''),(121,1,9513,'Takt',''),(122,1,9584,'Takt',''),(123,1,9652,'Takt',''),(124,1,9719,'Bar',''),(125,1,9787,'Takt',''),(126,1,9858,'Takt',''),(127,1,9926,'Takt',''),(128,1,9997,'Bar',''),(129,1,10065,'Takt',''),(130,1,10136,'Takt',''),(131,1,10201,'Takt',''),(132,1,10271,'Bar',''),(133,1,10340,'Takt',''),(134,1,10410,'Takt',''),(135,1,10479,'Takt',''),(136,1,10549,'Bar,Reloop',''),(137,1,10618,'Reloop,Takt',''),(138,1,10685,'Reloop,Takt',''),(139,1,10753,'Reloop,Takt',''),(140,1,10824,'Bar,Reloop',''),(141,1,10892,'Takt',''),(142,1,10963,'Takt',''),(143,1,11031,'Takt',''),(144,1,11102,'Bar',''),(145,1,11166,'Takt',''),(146,1,11237,'Takt',''),(147,1,11305,'Takt',''),(148,1,11376,'Bar',''),(149,1,11444,'Takt',''),(150,1,11515,'Takt',''),(151,1,11586,'Takt',''),(152,1,11650,'Bar',''),(153,1,11719,'Takt',''),(154,1,11789,'Takt',''),(155,1,11858,'Takt',''),(156,1,11928,'Bar',''),(157,1,11997,'Takt',''),(158,1,12068,'Takt',''),(159,1,12132,'Takt',''),(160,1,12203,'Bar',''),(161,1,12271,'Takt',''),(162,1,12342,'Takt',''),(163,1,12410,'Takt',''),(164,1,12481,'Bar',''),(165,1,12551,'Takt',''),(166,1,12616,'Takt',''),(167,1,12684,'Takt',''),(168,1,12755,'Bar',''),(169,1,12825,'Takt',''),(170,1,12894,'Takt',''),(171,1,12964,'Takt',''),(172,1,13033,'Bar,Velkommen','VELKOMMEN TIL DISTRICT'),(173,1,13099,'Takt',''),(174,1,13168,'Takt',''),(175,1,13239,'Takt,DistrictSlutt','DISTRICT'),(176,1,13307,'SluttSlag',''),(177,1,13378,'SluttTakt',''),(178,1,13446,'SluttTakt',''),(179,1,13517,'SluttTakt',''),(180,1,13581,'SluttSlag',''),(181,1,13652,'SluttTakt','');
/*!40000 ALTER TABLE `tc` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-09-04  7:05:00
