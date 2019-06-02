$('#toggle1').change(function(){
    var mode1= $(this).prop('checked');
    $.ajax({
        type:'POST',
        dataType:'JSON',
        url:'LEDS.php',
        data:'mode1='+mode1,
        success:function(data)
        {
            var data=eval(data);
            mensaje=data.mensaje;
            estado=data.estado;
            $("#heading1").html(estado);
        }
    });
});
$('#toggle2').change(function(){
    var mode2= $(this).prop('checked');
    $.ajax({
        type:'POST',
        dataType:'JSON',
        url:'LEDS.php',
        data:'mode2='+mode2,
        success:function(data)
        {
            var data=eval(data);
            mensaje=data.mensaje;
            estado=data.estado;
            $("#heading2").html(estado);
        }
    });
});
$('#toggle3').change(function(){
    var mode3= $(this).prop('checked');
    $.ajax({
        type:'POST',
        dataType:'JSON',
        url:'LEDS.php',
        data:'mode3='+mode3,
        success:function(data)
        {
            var data=eval(data);
            mensaje=data.mensaje;
            estado=data.estado;
            $("#heading3").html(estado);
        }
    });
});
$('#toggle4').change(function(){
    var mode4= $(this).prop('checked');
    $.ajax({
        type:'POST',
        dataType:'JSON',
        url:'LEDS.php',
        data:'mode4='+mode4,
        success:function(data)
        {
            var data=eval(data);
            mensaje=data.mensaje;
            estado=data.estado;
            $("#heading4").html(estado);
        }
    });
});
