/**
 * @file
 * Redeems coupon codes
 */
import { request } from '../API';
import { submitHandler } from '../Util/DOM';
import Analytics from '../Analytics';
import Messages from '../Screens/Elements/Messages';
import Sound from '../Sound';
import Icons from '../Screens/Elements/Icons';
import Session from '../Session/Session';

Screens.add('redeem', () => {
  Screens.update('redeem', `
    <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
    <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
    <div class="menu">
      <form id="private-join">
        <input type="text" id="code" value="" placeholder="Enter code" style="margin-bottom: 1em;">
        <button class="box" type="submit" id="redeem">REDEEM</a>
        <input type="submit" style="visibility: hidden; position: absolute;" />
      </form>
    </div>
  `);

  $('#code').focus();
  submitHandler('redeem', false, () => {
    Analytics.event('Code', 'redeem');
    request('app/redeem', { code: $('#code').value }, (response) => {
      const account = Session.getAccount();
      account.currency.current += parseInt(response.cordium, 10);
      Session.setAccount(account);
      Messages.show(response.message);
      Sound.play('win');
      Screens.show('home');
    }, () => {
      $('#redeem').classList.remove('waiting');
    });
  });
});
