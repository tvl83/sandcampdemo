<?php
/**
 * Created by PhpStorm.
 * User: vnc
 * Date: 1/13/14
 * Time: 7:49 AM
 */

define('DRUPAL_ROOT', getcwd());
require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
require_once DRUPAL_ROOT . '/sites/all/modules/custom/demo/Num.class.inc';
drupal_override_server_variables();
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

/**
 * Class NumTest
 */
class NumTest extends PHPUnit_Framework_TestCase {

  /**
   * Test whether getNextInteger() function works well with floats.
   */
  public function testFloat() {
    $numClass = new Num(3.4);
    $this->assertEquals($numClass->getNextInteger(), 4);
  }

  /**
   * Test whether getNextInteger() function works well with integers.
   */
  public function testInteger() {
    $numClass = new Num(2);
    $this->assertEquals($numClass->getNextInteger(), 3);
  }

  /**
   * Test whether getNextInteger() function works well with negative numbers.
   */
  public function testNegative() {
    $numClass = new Num(-1.2);
    $this->assertEquals($numClass->getNextInteger(), -2);
  }

}
