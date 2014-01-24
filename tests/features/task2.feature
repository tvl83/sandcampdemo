Feature: Demo-2 Next integer page access
When a user visits the next integer page
as an authenticated user
I should not be able to access the page

  @javascript @insulated @1
  Scenario: Admin user should be able to view the page and the next integer
    Given I am on "/?q=user/login"
    When I fill in "Username" with "neerav"
    When I fill in "Password" with "1234"
    When I press button "Log in"
    Then I should be redirected to "?q=users/neerav"
    Given I am on "/?q=show-next-integer/3.4"
    Then I should see "Access denied"
    Then I click "Log out"