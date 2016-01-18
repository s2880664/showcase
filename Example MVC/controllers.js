angular.module('starter.controllers', [])

// Formulator Controllers
.controller('HerbsCtrl', function($scope, $rootScope, $location, HerbsService, SearchDataService, DBA, ActionSearchDataService, HerbsDataService, $cordovaSQLite, $ionicLoading, $cordovaToast) {

  var searchData;

  //Actions List Options
  $scope.shouldShowDelete = true;
  $scope.shouldShowReorder = true;
  $scope.listCanSwipe = true;

  //Default all checkboxs to false
  $scope.herbalExtract = false;
  $scope.mediherb = false
  $scope.optimalRx = false;
  $scope.nutritioncare = false;
  $scope.all = false;

  //Needed for box opening
  $scope.open = 0;

  $scope.actions = [];
  $scope.herbsArr = [];
  $scope.herbsArr2 = [];
  $scope.items = [];

  //refresh screen so actions show up
  $rootScope.$on( "$ionicView.enter", function( scopes, states ) {
    $scope.actions = ActionSearchDataService.get();
    /*$scope.actions = [{actionID: '1', action: 'Adaptogen'},
    {actionID: '2', action: 'Adrenal Tonic'},
    {actionID: '3', action: 'Alterative'},
    {actionID: '4', action: 'Analgesic'},
    {actionID: '5', action: 'Anodyne'}];
    $scope.herbsArr = HerbsDataService.getDummyData();*/
  });

  //Gets which options are ticked in Search by Brand
  $scope.storeScopeData = function()
  {
    searchData = [];

    var actionSearchData = ActionSearchDataService.get();
    var pushData;

    for (var i = 0; i < actionSearchData.length; i++) {
      pushData = {subject: 'action', value: actionSearchData[i].actionID};
      searchData.push(pushData);
    }

    if($scope.all == true){
      searchData.push({subject: 'brand', value: '1'});
      searchData.push({subject: 'brand', value: '2'});
      searchData.push({subject: 'brand', value: '3'});
      searchData.push({subject: 'brand', value: '4'});
    }
    else
    {
      if($scope.herbalExtract == true){
        searchData.push({subject: 'brand', value: '2'});
      }
      if($scope.mediherb == true){
        searchData.push({subject: 'brand', value: '1'});
      }
      if($scope.optimalRx == true){
        searchData.push({subject: 'brand', value: '3'});
      }
      if($scope.nutritioncare == true){
        searchData.push({ subject:'brand',  value:'4'});
      }
    }
    //Stores search data for other controllers to access
    SearchDataService.set(searchData);
  }
  $scope.removeAction = function(action){
    ActionSearchDataService.remove(action);
    $cordovaToast.show('Removed '+ action.action + ' from search', 'short', 'bottom');
  }

  $scope.addHerb = function(herb){
    HerbsDataService.add(herb);
    $cordovaToast.show('Added '+ herb.commonName + ' to the list', 'short', 'bottom');
  };

  $scope.paginationClick = function(upOrDown){
    if(upOrDown == "up"){
      $scope.pageNumber++;
      $scope.updateSearchListLoading();
    };
    if (upOrDown == "down" && $scope.pageNumber > 0) {
      $scope.pageNumber--;
      $scope.updateSearchListLoading();
    };
  }
  $scope.updateSearchListButton = function(){
    $scope.pageNumber = 0;
    $scope.updateSearchListLoading();
  }

  $scope.updateSearchListLoading = function(){
    if ($scope.all || $scope.herbalExtract || $scope.mediherb || $scope.optimalRx || $scope.nutritioncare) {
      $scope.loadingIndicator = $ionicLoading.show({
        content: 'Loading...',
        showBackdrop: true
      });
      $scope.updateSearchList();
    } else {
      $cordovaToast.show('Please select at lease one Brand', 'short', 'center');
    };
  }

  $scope.updateSearchList = function()
  {
    if(!$scope.nameInput)
    {
      //Update Datas
      $scope.storeScopeData();
      $scope.actions = ActionSearchDataService.get();

      var searchParameters = SearchDataService.get();

      var brandPresent = false;
      var actionPresent = false;
      var query = "SELECT DISTINCT brands.brand, herbs.herbID, herbs.brandID, herbs.pregnancyCategory, herbs.lactationCategory, herbs.commonName, herbs.latinBinomialName, herbs.ratio, herbs.weeklyDosageRange FROM herbs, brands";
      var query2 = "SELECT DISTINCT herbs.herbID FROM herbs, brands";
      var actions = [];
      var brands = [];

      for(i = 0; i < searchParameters.length; i++){
        if(searchParameters[i].subject == "action")
        {
          //add actions to a seperate array for later
          actions.push(searchParameters[i].value);
          actionPresent = true;
        }
        if(searchParameters[i].subject == "brand")
        {
          //add brands to a seperate array for later
          brands.push(searchParameters[i].value);
          brandPresent = true;
        }
      }
      if(actionPresent)
      {
        query += (" INNER JOIN action_lists ON action_lists.herbID = herbs.herbID WHERE action_lists.actionID IN (");
        query2 += (" INNER JOIN action_lists ON action_lists.herbID = herbs.herbID WHERE action_lists.actionID IN (");
        var commas = false;
        for(i = 0; i < actions.length; i++)
        {
          if(!commas)
          {
            query += (actions[i]);
            query2 += (actions[i]);
            commas = true;
          }
          else
          {
            query += (',');
            query += (actions[i]);
            query2 += (',');
            query2 += (actions[i]);
          }
        }
        query += (')');
        query2 += (')');
      }
      if(brandPresent && actionPresent)
      {
        query += (" AND herbs.brandID IN (");
        query2 += (" AND herbs.brandID IN (");
        commas = false;
        for(i = 0; i < brands.length; i++)
        {
          if(!commas)
          {
            query += (brands[i]);
            query2 += (brands[i]);
            commas = true;
          }
          else
          {
            query += (',');
            query += (brands[i]);
            query2 += (',');
            query2 += (brands[i]);
          }
        }
        query += (')');
        query2 += (')');
      }
      if(brandPresent && !actionPresent)
      {
        query += (" WHERE herbs.brandID IN (");
        query2 += (" WHERE herbs.brandID IN (");
        for(i = 0; i < brands.length; i++)
        {
          if(i == 0)
          {
            query += (brands[i]);
            query2 += (brands[i]);
          }
          else
          {
            query += (',');
            query += (brands[i]);
            query2 += (',');
            query2 += (brands[i]);
          }
        }
        query += (')');
        query2 += (')');
      }
      query += (' AND herbs.brandID = brands.brandID LIMIT 10 OFFSET ' + $scope.pageNumber * 10 + ';');
      query2 += (' AND herbs.brandID = brands.brandID;');

      // Store pregnancy categorys + description for later below
      var pregnancyCategorysArr;
      HerbsService.getAllPregnancyCategorys().then(function(result){
        pregnancyCategorysArr = result;
      });
      // Store lactation categorys + description for later below
      var lactationCategorysArr;
      HerbsService.getAllLactationCategorys().then(function(result){
        lactationCategorysArr = result;
      });

      DBA.query(query).then(function(result)
      {
        $scope.herbsArr = DBA.getAll(result);
        for(i = 0; i < $scope.herbsArr.length; i++)
        {
          HerbsDataService.addActions($scope.herbsArr[i]);

          if($scope.herbsArr[i].pregnancyCategory != null)
          {
            // Loop though pregnancyCategorysArr and find matching description for current items pregnancy category
            for (var j = 0; j < pregnancyCategorysArr.length; j++) {
              if($scope.herbsArr[i].pregnancyCategory == pregnancyCategorysArr[j].pregnancyCategory){
                $scope.herbsArr[i]["pregnancyDescription"] = pregnancyCategorysArr[j].description;
              }
            };
          }
          if($scope.herbsArr[i].lactationCategory != null)
          {
            // Loop though lactationCategorysArr and find matching description for current items lactation category
            for (var j = 0; j < lactationCategorysArr.length; j++) {
              if($scope.herbsArr[i].lactationCategory == lactationCategorysArr[j].lactationCategory){
                $scope.herbsArr[i]["lactationDescription"] = lactationCategorysArr[j].description;
              }
            };
          }
        }
        console.log(pregnancyCategorysArr);
        console.log($scope.herbsArr);
        // Hide loading screen
        $ionicLoading.hide();
      });
      DBA.query(query2).then(function(result)
      {
        $scope.herbsArr2 = DBA.getAll(result);
        $scope.resultCount = $scope.herbsArr2.length;
        console.log($scope.resultCount);
      });
    } else {
      //Update Datas
      $scope.storeScopeData();

      var searchParameters = SearchDataService.get();

      var query = "SELECT DISTINCT brands.brand, herbs.herbID, herbs.brandID, herbs.pregnancyCategory, herbs.lactationCategory, herbs.commonName, herbs.latinBinomialName, herbs.ratio, herbs.weeklyDosageRange FROM herbs, brands";
      var query2 = "SELECT DISTINCT herbs.herbID FROM herbs, brands";

      var brands = [];

      for(i = 0; i < searchParameters.length; i++){
        if(searchParameters[i].subject == "brand")
        {
          //add brands to a seperate array for later
          brands.push(searchParameters[i].value);
        }
      }
      query += (" WHERE herbs.brandID IN (");
      query2 += (" WHERE herbs.brandID IN (");
      for(i = 0; i < brands.length; i++)
      {
        if(i == 0)
        {
          query += (brands[i]);
          query2 += (brands[i]);
        }
        else
        {
          query += (',');
          query += (brands[i]);
          query2 += (',');
          query2 += (brands[i]);
        }
      }
      query += (')');
      query += (" AND herbs.commonName LIKE '%" + $scope.nameInput + "%'");
      query += (' AND herbs.brandID = brands.brandID LIMIT 10 OFFSET ' + $scope.pageNumber * 10 + ';');

      query2 += (')');
      query2 += (" AND herbs.commonName LIKE '%" + $scope.nameInput + "%'");
      query2 += (' AND herbs.brandID = brands.brandID;');

      // Store pregnancy categorys + description for later below
      var pregnancyCategorysArr;
      HerbsService.getAllPregnancyCategorys().then(function(result){
        pregnancyCategorysArr = result;
      });
      // Store lactation categorys + description for later below
      var lactationCategorysArr;
      HerbsService.getAllLactationCategorys().then(function(result){
        lactationCategorysArr = result;
      });

      DBA.query(query).then(function(result){
        $scope.herbsArr = DBA.getAll(result);
        for(i = 0; i < $scope.herbsArr.length; i++)
        {
          HerbsDataService.addActions($scope.herbsArr[i]);

          if($scope.herbsArr[i].pregnancyCategory != null)
          {
            // Loop though pregnancyCategorysArr and find matching description for current items pregnancy category
            for (var j = 0; j < pregnancyCategorysArr.length; j++) {
              if($scope.herbsArr[i].pregnancyCategory == pregnancyCategorysArr[j].pregnancyCategory){
                $scope.herbsArr[i]["pregnancyDescription"] = pregnancyCategorysArr[j].description;
              }
            };
          }
          if($scope.herbsArr[i].lactationCategory != null)
          {
            // Loop though lactationCategorysArr and find matching description for current items lactation category
            for (var j = 0; j < lactationCategorysArr.length; j++) {
              if($scope.herbsArr[i].lactationCategory == lactationCategorysArr[j].lactationCategory){
                $scope.herbsArr[i]["lactationDescription"] = lactationCategorysArr[j].description;
              }
            };
          }
        }
        console.log(pregnancyCategorysArr);
        console.log($scope.herbsArr);
        // Hide loading screen
        $ionicLoading.hide();
      });

      DBA.query(query2).then(function(result)
      {
        $scope.herbsArr2 = DBA.getAll(result);
        $scope.resultCount = $scope.herbsArr2.length;
      });
    }
  }
});
