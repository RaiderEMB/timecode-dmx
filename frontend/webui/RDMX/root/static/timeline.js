$(document).ready(function () {
	$('#generate_show').click(function () {
		var obj = this;
		obj.disabled = true;
		$.ajax({
			url: window.location.href + '/generate',
			success: function() {
				obj.disabled = false;
			}
		});
	});
});
