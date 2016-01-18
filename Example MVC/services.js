angular.module('starter.services', [])

//Database Access
.factory('DBA', function($cordovaSQLite, $q, $ionicPlatform) {
  var self = this;

  // Handle query's and potential errors
  self.query = function (query, parameters) {
    console.log(query);
    parameters = parameters || [];
    var q = $q.defer();

    $ionicPlatform.ready(function () {
      $cordovaSQLite.execute(herbsDatabase, query, parameters)
      .then(function (result) {
        q.resolve(result);
      }, function (error) {
        console.warn('I found an error');
        console.warn(error);
        alert(error.message);
        q.reject(error);
      });
    });
    return q.promise;
  }

  // Proces a result set
  self.getAll = function(result) {
    var output = [];
    for (var i = 0; i < result.rows.length; i++) {
      output.push(result.rows.item(i));
    }
    return output;
  }

  // Proces a single result
  self.getById = function(result) {
    var output = null;
    output = angular.copy(result.rows.item(0));
    return output;
  })

  //A service for the herbs tab only
  .factory('HerbsService', function($cordovaSQLite, DBA) {
    var self = this;

    self.getHerb = function(herbID) {
      var parameters = [herbID];
      return DBA.query("SELECT herbID, brandID, actionListID, interactionListID, pregnancyCategory, lactationCategory, commonName, latinBinomialName, ratio, weeklyDosageRange FROM herbs WHERE herbID = (?)", parameters)
      .then(function(result) {
        return DBA.getById(result);
      });
    }

    self.getActions = function(herbID) {
      var parameters = [herbID];
      return DBA.query("SELECT action FROM action_lists INNER JOIN actions ON action_lists.actionID = actions.actionID WHERE action_lists.herbID = (?)", parameters)
      .then(function(result) {
        return DBA.getById(result);
      });
    }

    self.getBrand = function(brandID) {
      var parameters = [brandID];
      return DBA.query("SELECT brand FROM brands WHERE brandID = (?)", parameters)
      .then(function(result) {
        return DBA.getById(result);
      });
    }

    self.getAllPregnancyCategorys = function() {
      return DBA.query("SELECT pregnancyCategory, description FROM pregnancy_categorys")
      .then(function(result) {
        return DBA.getAll(result);
      });
    }

    self.getAllLactationCategorys = function() {
      return DBA.query("SELECT lactationCategory, description FROM lactation_categorys")
      .then(function(result) {
        return DBA.getAll(result);
      });
    }

    self.searchLike = function(table, where, strIn) {
      var parameters = [table, where, strIn];
      return DBA.query("SELECT * FROM (?) WHERE (?) = (?)", parameters)
      .then(function(result) {
        return DBA.getAll(result);
      });
    }
    return self;
  });
