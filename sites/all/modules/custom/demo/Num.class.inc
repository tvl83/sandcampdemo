<?php
/**
 * Created by PhpStorm.
 * User: vnc
 * Date: 1/13/14
 * Time: 7:36 AM
 */

class Num {

  private $num;

  public function __construct($num) {
    $this->num = $num;
  }

  public function getNextInteger() {
    return (floor($this->num) + 1);
  }

} 