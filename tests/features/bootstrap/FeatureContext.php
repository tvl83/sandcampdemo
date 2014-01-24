<?php

use Behat\Behat\Context\ClosuredContextInterface,
    Behat\Behat\Context\TranslatedContextInterface,
    Behat\Behat\Context\BehatContext,
    Behat\Behat\Exception\PendingException;
use Behat\Gherkin\Node\PyStringNode,
    Behat\Gherkin\Node\TableNode;
use Behat\MinkExtension\Context\MinkContext;

//
// Require 3rd-party libraries here:
//
//   require_once 'PHPUnit/Autoload.php';
//   require_once 'PHPUnit/Framework/Assert/Functions.php';
//

/**
 * Features context.
 */
class FeatureContext extends MinkContext
{
    /**
     * Initializes context.
     * Every scenario gets it's own context object.
     *
     * @param array $parameters context parameters (set them up through behat.yml)
     */
    public function __construct(array $parameters)
    {
        // Initialize your context here
    }


//
// Place your definition and hook methods here:
//
//    /**
//     * @Given /^I have done something with "([^"]*)"$/
//     */
//    public function iHaveDoneSomethingWith($argument)
//    {
//        doSomethingWith($argument);
//    }
//


    /**
     * @Then /^I click "([^"]*)"$/
     */
    public function iClick($link)
    {
        $link = $this->fixStepArgument($link);
        $this->getSession()->getPage()->clickLink($link);

    }


    /**
     * @Then /^I press button "([^"]*)"$/
     */
    public function iPressButton($button)
    {
        $button = $this->fixStepArgument($button);
        $this->getSession()->getPage()->pressButton($button);
    }


    /**
     * @Then /^I select option "([^"]*)"$/
     */
    public function iSelectOption($select, $option)
    {
        $select = $this->fixStepArgument($select);
        $option = $this->fixStepArgument($option);
        $this->getSession()->getPage()->selectFieldOption($select, $option);
    }


    /**
     * @Then /^I should be redirected to "([^"]*)"$/
     */
    public function iShouldBeRedirectedTo($arg1)
    {
        $this->getSession();//->wait(10000);
        $this->assertPageAddress($arg1);
    }


    /**
     * Click some text
     *
     * @When /^I click on the text "([^"]*)"$/
     */
    public function iClickOnTheText($text)
    {
        $session = $this->getSession();
        $element = $session->getPage()->find(
            'xpath',
            $session->getSelectorsHandler()->selectorToXpath('xpath', '*//*[text()="'. $text .'"]')
        );
        if (null === $element) {
            throw new \InvalidArgumentException(sprintf('Cannot find text: "%s"', $text));
        }

        $element->click();

    }


    /**
     * @Then /^I click button "([^"]*)"$/
     */
    public function iClickButton($button)
    {
        $page = $this->getSession()->getPage();
        $element = $page->find(
            'xpath',
            $session->getSelectorsHandler()->selectorToXpath('xpath', '//input[@value="'. $button .'"]')
        );
        $element->doubleclick();
    }
}
