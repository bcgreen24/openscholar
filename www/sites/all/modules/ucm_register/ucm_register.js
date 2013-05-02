$(document).ready(function() {
  
  if (window.location.hash == '#noaccess') {
    $('body').prepend('<div id="top-error">This site is only available to faculty members.</div>');

    var height = $('#top-error').outerHeight(true);
    $('#top-error').css({marginTop : '-' + height + 'px'});
    $('#top-error').animate({marginTop: '0px'}, 1000, function () {
      setTimeout(function() { $('#top-error').animate({marginTop: '-' + height + 'px'}, 1000)}, 5000);
    });
  }
});
